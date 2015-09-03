/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <unistd.h> /* for usleep() */
#include <glib.h>

#include <ocstack.h>
#include <octypes.h>
#include <ocpayload.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icd.h"
#include "icd-payload.h"
#include "icd-dbus.h"
#include "icd-ioty.h"
#include "icd-ioty-type.h"
#include "icd-ioty-ocprocess.h"

static int icd_ioty_alive;

typedef int (*_ocprocess_cb)(void *user_data);

struct icd_ioty_worker
{
	void *ctx;
	_ocprocess_cb cb;
};


struct icd_req_context {
	unsigned int signum;
	char *bus_name;
	int types;
	int observer_id;
	int observe_action;
	OCRequestHandle request_h;
	OCResourceHandle resource_h;
	GVariant *payload;
	GVariantBuilder *options;
	GVariantBuilder *query;
};


struct icd_find_context {
	unsigned int signum;
	char *bus_name;
	int conn_type;
	GVariant **payload;
};


struct icd_crud_context {
	int res;
	int crud_type;
	GVariant *payload;
	GVariantBuilder *options;
	GDBusMethodInvocation *invocation;
};


struct icd_info_context {
	unsigned int signum;
	int info_type;
	char *bus_name;
	GVariant *payload;
};


struct icd_observe_context {
	unsigned int signum;
	int res;
	int seqnum;
	char *bus_name;
	GVariant *payload;
	GVariantBuilder *options;
};


struct icd_presence_context {
	unsigned int signum;
	char *bus_name;
	int result;
	unsigned int nonce;
	OCDevAddr *dev_addr;
};


void icd_ioty_ocprocess_stop()
{
	icd_ioty_alive = 0;
}


static void* _ocprocess_worker_thread(void *data)
{
	int ret;
	struct icd_ioty_worker *worker = data;

	if (NULL == data) {
		ERR("worker is NULL");
		return NULL;
	}

	ret = worker->cb(worker->ctx);
	if (IOTCON_ERROR_NONE != ret)
		ERR("cb() Fail(%d)", ret);

	/* worker was allocated from _ocprocess_worker_start() */
	free(worker);

	/* GCC warning happen if use g_thread_exit() */
	return NULL;
}


static int _ocprocess_worker_start(_ocprocess_cb cb, void *ctx)
{
	GError *error;
	GThread *thread;
	struct icd_ioty_worker *worker;

	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	worker = calloc(1, sizeof(struct icd_ioty_worker));
	if (NULL == worker) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	worker->cb = cb;
	worker->ctx = ctx;

	/* TODO : consider thread pool mechanism */
	thread = g_thread_try_new("worker_thread", _ocprocess_worker_thread, worker, &error);
	if (NULL == thread) {
		ERR("g_thread_try_new() Fail(%s)", error->message);
		g_error_free(error);
		free(worker);
		return IOTCON_ERROR_SYSTEM;
	}

	/* DO NOT join thread. It was already detached by calling g_thread_unref() */
	g_thread_unref(thread);

	/* DO NOT FREE worker. It MUST be freed in the _ocprocess_worker_thread() */

	return IOTCON_ERROR_NONE;
}


static int _ocprocess_response_signal(const char *dest, const char *signal,
		unsigned int signum, GVariant *value)
{
	int ret;
	char sig_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	ret = snprintf(sig_name, sizeof(sig_name), "%s_%u", signal, signum);
	if (ret <= 0 || sizeof(sig_name) <= ret) {
		ERR("snprintf() Fail(%d)", ret);
		return IOTCON_ERROR_IO_ERROR;
	}

	ret = icd_dbus_emit_signal(dest, sig_name, value);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_dbus_emit_signal() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


static inline GVariantBuilder* _ocprocess_parse_header_options(
		OCHeaderOption *oic_option, int option_size)
{
	int i;
	GVariantBuilder *options;

	options = g_variant_builder_new(G_VARIANT_TYPE("a(qs)"));
	for (i = 0; i < option_size; i++) {
		g_variant_builder_add(options, "(qs)", oic_option[i].optionID,
				oic_option[i].optionData);
	}

	return options;
}


static int _worker_req_handler(void *context)
{
	int ret;
	GVariant *value;
	struct icd_req_context *ctx = context;
	GVariantBuilder payload_builder;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	g_variant_builder_init(&payload_builder, G_VARIANT_TYPE("av"));
	if (ctx->payload)
		g_variant_builder_add(&payload_builder, "v", ctx->payload);

	value = g_variant_new("(ia(qs)a(ss)iiavxx)",
			ctx->types,
			ctx->options,
			ctx->query,
			ctx->observe_action,
			ctx->observer_id,
			&payload_builder,
			ICD_POINTER_TO_INT64(ctx->request_h),
			ICD_POINTER_TO_INT64(ctx->resource_h));

	ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_REQUEST_HANDLER,
			ctx->signum, value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	free(ctx->bus_name);
	g_variant_builder_unref(ctx->options);
	g_variant_builder_unref(ctx->query);
	free(ctx);

	return ret;
}


OCEntityHandlerResult icd_ioty_ocprocess_req_handler(OCEntityHandlerFlag flag,
		OCEntityHandlerRequest *request, void *user_data)
{
	FN_CALL;
	int ret;
	unsigned int signal_number;
	char *query_str, *query_key, *query_value;
	char *token, *save_ptr1, *save_ptr2;
	char *bus_name = NULL;
	struct icd_req_context *req_ctx;

	RETV_IF(NULL == request, OC_EH_ERROR);

	req_ctx = calloc(1, sizeof(struct icd_req_context));
	if (NULL == req_ctx) {
		ERR("calloc() Fail(%d)", errno);
		return OC_EH_ERROR;
	}

	/* handle */
	req_ctx->request_h = request->requestHandle;
	req_ctx->resource_h = request->resource;

	ret = icd_dbus_client_list_get_info(req_ctx->resource_h, &signal_number, &bus_name);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_dbus_client_list_get_info() Fail(%d)", ret);
		free(req_ctx);
		return OC_EH_ERROR;
	}

	/* signal number & bus_name */
	req_ctx->signum = signal_number;
	req_ctx->bus_name = bus_name;

	/* request type */
	if (OC_REQUEST_FLAG & flag) {
		switch (request->method) {
		case OC_REST_GET:
			req_ctx->types = IOTCON_REQUEST_GET;
			req_ctx->payload = NULL;

			if (OC_OBSERVE_FLAG & flag) {
				req_ctx->types |= IOTCON_REQUEST_OBSERVE;
				/* observation info*/
				req_ctx->observer_id = request->obsInfo.obsId;
				req_ctx->observe_action = request->obsInfo.action;
			}
			break;
		case OC_REST_PUT:
			req_ctx->types = IOTCON_REQUEST_PUT;
			req_ctx->payload = icd_payload_to_gvariant(request->payload);
			break;
		case OC_REST_POST:
			req_ctx->types = IOTCON_REQUEST_POST;
			req_ctx->payload = icd_payload_to_gvariant(request->payload);
			break;
		case OC_REST_DELETE:
			req_ctx->types = IOTCON_REQUEST_DELETE;
			req_ctx->payload = NULL;
			break;
		default:
			free(req_ctx->bus_name);
			free(req_ctx);
			return OC_EH_ERROR;
		}
	}

	/* header options */
	req_ctx->options = _ocprocess_parse_header_options(
			request->rcvdVendorSpecificHeaderOptions,
			request->numRcvdVendorSpecificHeaderOptions);

	/* query */
	req_ctx->query = g_variant_builder_new(G_VARIANT_TYPE("a(ss)"));
	query_str = request->query;
	while ((token = strtok_r(query_str, "&;", &save_ptr1))) {
		while ((query_key = strtok_r(token, "=", &save_ptr2))) {
			token = NULL;
			query_value = strtok_r(token, "=", &save_ptr2);
			if (NULL == query_value)
				break;

			g_variant_builder_add(req_ctx->query, "(ss)", query_key, query_value);
		}
		query_str = NULL;
	}

	ret = _ocprocess_worker_start(_worker_req_handler, req_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		free(req_ctx->bus_name);
		if (req_ctx->payload)
			g_variant_unref(req_ctx->payload);
		g_variant_builder_unref(req_ctx->options);
		g_variant_builder_unref(req_ctx->query);
		free(req_ctx);
		return OC_EH_ERROR;
	}

	/* DO NOT FREE req_ctx. It MUST be freed in the _worker_req_handler func */

	return OC_EH_OK;
}


gpointer icd_ioty_ocprocess_thread(gpointer data)
{
	FN_CALL;
	OCStackResult result;

	icd_ioty_alive = 1;
	while (icd_ioty_alive) {
		icd_ioty_csdk_lock();
		result = OCProcess();
		icd_ioty_csdk_unlock();
		if (OC_STACK_OK != result) {
			ERR("OCProcess() Fail(%d)", result);
			break;
		}

		/* TODO : SHOULD revise time or usleep */
		usleep(10);
	}

	return NULL;
}


static int _worker_find_cb(void *context)
{
	GVariant *value;
	int i, ret;
	struct icd_find_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	for (i = 0; ctx->payload[i]; i++) {
		value = g_variant_new("(vi)", ctx->payload[i], ctx->conn_type);
		/* TODO : If one device has multi resources, it comes as bulk data.
		 * To reduce the number of emit_signal, let's send signal only one time for one device.
		 * for ex, client list. */
		ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_FOUND_RESOURCE,
				ctx->signum, value);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_ocprocess_response_signal() Fail(%d)", ret);
			g_variant_unref(value);
			return ret;
		}
	}

	/* ctx was allocated from icd_ioty_ocprocess_find_cb() */
	free(ctx->bus_name);
	free(ctx);

	return ret;
}


OCStackApplicationResult icd_ioty_ocprocess_find_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	int ret;
	struct icd_find_context *find_ctx;
	icd_sig_ctx_s *sig_context = ctx;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	if (NULL == resp->payload)
		/* normal case : payload COULD be NULL */
		return OC_STACK_KEEP_TRANSACTION;
	RETVM_IF(PAYLOAD_TYPE_DISCOVERY != resp->payload->type,
			OC_STACK_KEEP_TRANSACTION, "Invalid payload type(%d)", resp->payload->type);

	find_ctx = calloc(1, sizeof(struct icd_find_context));
	if (NULL == find_ctx) {
		ERR("calloc() Fail(%d)", errno);
		return OC_STACK_KEEP_TRANSACTION;
	}

	find_ctx->signum = sig_context->signum;
	find_ctx->bus_name = ic_utils_strdup(sig_context->bus_name);
	find_ctx->payload = icd_payload_res_to_gvariant(resp->payload, &resp->devAddr);
	find_ctx->conn_type = icd_ioty_transport_flag_to_conn_type(resp->devAddr.adapter,
			resp->devAddr.flags);

	ret = _ocprocess_worker_start(_worker_find_cb, find_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		free(find_ctx->bus_name);
		ic_utils_gvariant_array_free(find_ctx->payload);
		free(find_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* DO NOT FREE sig_context. It MUST be freed in the ocstack */
	/* DO NOT FREE find_ctx. It MUST be freed in the _worker_find_cb func */

	return OC_STACK_KEEP_TRANSACTION;
}


static int _worker_crud_cb(void *context)
{
	GVariant *value;
	struct icd_crud_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	if (ICD_CRUD_DELETE == ctx->crud_type)
		value = g_variant_new("(a(qs)i)", ctx->options, ctx->res);
	else
		value = g_variant_new("(a(qs)vi)", ctx->options, ctx->payload, ctx->res);
	icd_ioty_complete(ctx->crud_type, ctx->invocation, value);

	/* ctx was allocated from icd_ioty_ocprocess_xxx_cb() */
	g_variant_builder_unref(ctx->options);
	free(ctx);

	return IOTCON_ERROR_NONE;
}


static int _worker_info_cb(void *context)
{
	int ret;
	const char *sig_name = NULL;
	struct icd_info_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	if (ICD_DEVICE_INFO == ctx->info_type)
		sig_name = IC_DBUS_SIGNAL_DEVICE;
	else if (ICD_PLATFORM_INFO == ctx->info_type)
		sig_name = IC_DBUS_SIGNAL_PLATFORM;

	ret = _ocprocess_response_signal(ctx->bus_name, sig_name, ctx->signum, ctx->payload);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	/* ctx was allocated from icd_ioty_ocprocess_info_cb() */
	free(ctx->bus_name);
	free(ctx);

	return ret;
}


static int _ocprocess_worker(_ocprocess_cb cb, int type, OCPayload *payload, int res,
		GVariantBuilder *options, void *ctx)
{
	int ret;
	struct icd_crud_context *crud_ctx;

	crud_ctx = calloc(1, sizeof(struct icd_crud_context));
	if (NULL == crud_ctx) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	crud_ctx->crud_type = type;
	crud_ctx->payload = icd_payload_to_gvariant(payload);
	crud_ctx->res = res;
	crud_ctx->options = options;
	crud_ctx->invocation = ctx;

	ret = _ocprocess_worker_start(cb, crud_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		if (crud_ctx->payload)
			g_variant_unref(crud_ctx->payload);
		g_variant_builder_unref(crud_ctx->options);
		free(crud_ctx);
	}

	/* DO NOT FREE crud_ctx. It MUST be freed in the _worker_crud_cb func */

	return ret;
}


OCStackApplicationResult icd_ioty_ocprocess_get_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret, res;
	OCStackResult result;
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_GET, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	result = resp->result;
	if (OC_STACK_OK == result) {
		res = IOTCON_RESPONSE_RESULT_OK;
		options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
				resp->numRcvdVendorSpecificHeaderOptions);
	} else {
		WARN("resp error(%d)", result);
		res = IOTCON_RESPONSE_RESULT_ERROR;
		options = NULL;
	}

	ret = _ocprocess_worker(_worker_crud_cb, ICD_CRUD_GET, resp->payload, res,
			options, ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker() Fail(%d)", ret);
		icd_ioty_complete_error(ICD_CRUD_GET, ctx, ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	return OC_STACK_DELETE_TRANSACTION;
}


OCStackApplicationResult icd_ioty_ocprocess_put_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret, res;
	OCStackResult result;
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_PUT, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	result = resp->result;
	switch (result) {
	case OC_STACK_OK:
		res = IOTCON_RESPONSE_RESULT_OK;
		break;
	case OC_STACK_RESOURCE_CREATED:
		res = IOTCON_RESPONSE_RESULT_RESOURCE_CREATED;
		break;
	default:
		WARN("resp error(%d)", result);
		res = IOTCON_RESPONSE_RESULT_ERROR;
		options = NULL;
	}

	if (IOTCON_RESPONSE_RESULT_ERROR != res) {
		options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
				resp->numRcvdVendorSpecificHeaderOptions);
	}

	ret = _ocprocess_worker(_worker_crud_cb, ICD_CRUD_PUT, resp->payload, res,
			options, ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker() Fail(%d)", ret);
		icd_ioty_complete_error(ICD_CRUD_PUT, ctx, ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	return OC_STACK_DELETE_TRANSACTION;
}


OCStackApplicationResult icd_ioty_ocprocess_post_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret, res;
	OCStackResult result;
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_POST, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	result = resp->result;
	switch (result) {
	case OC_STACK_OK:
		res = IOTCON_RESPONSE_RESULT_OK;
		break;
	case OC_STACK_RESOURCE_CREATED:
		res = IOTCON_RESPONSE_RESULT_RESOURCE_CREATED;
		break;
	default:
		WARN("resp error(%d)", result);
		res = IOTCON_RESPONSE_RESULT_ERROR;
		options = NULL;
	}

	if (IOTCON_RESPONSE_RESULT_ERROR != res) {
		options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
				resp->numRcvdVendorSpecificHeaderOptions);
	}

	ret = _ocprocess_worker(_worker_crud_cb, ICD_CRUD_POST, resp->payload, res,
			options, ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker() Fail(%d)", ret);
		icd_ioty_complete_error(ICD_CRUD_POST, ctx, ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	return OC_STACK_DELETE_TRANSACTION;
}


OCStackApplicationResult icd_ioty_ocprocess_delete_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret, res;
	OCStackResult result;
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_DELETE, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	result = resp->result;
	switch (result) {
	case OC_STACK_OK:
		res = IOTCON_RESPONSE_RESULT_OK;
		break;
	case OC_STACK_RESOURCE_DELETED:
		res = IOTCON_RESPONSE_RESULT_RESOURCE_DELETED;
		break;
	default:
		WARN("resp error(%d)", result);
		res = IOTCON_RESPONSE_RESULT_ERROR;
		options = NULL;
	}

	if (IOTCON_RESPONSE_RESULT_ERROR != res) {
		options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
				resp->numRcvdVendorSpecificHeaderOptions);
	}

	ret = _ocprocess_worker(_worker_crud_cb, ICD_CRUD_DELETE, NULL, res, options, ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker() Fail(%d)", ret);
		icd_ioty_complete_error(ICD_CRUD_DELETE, ctx, ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	return OC_STACK_DELETE_TRANSACTION;
}


static int _worker_observe_cb(void *context)
{
	int ret;
	GVariant *value;
	struct icd_observe_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_variant_new("(a(qs)vii)", ctx->options, ctx->payload, ctx->res,
			ctx->seqnum);

	ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_OBSERVE, ctx->signum,
			value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	/* ctx was allocated from icd_ioty_ocprocess_observe_cb() */
	free(ctx->bus_name);
	g_variant_builder_unref(ctx->options);
	free(ctx);

	return ret;
}


static void _observe_cb_response_error(const char *dest, unsigned int signum, int ret_val)
{
	int ret;
	GVariant *value;

	value = g_variant_new("(a(qs)vii)", NULL, NULL, ret_val, 0);

	ret = _ocprocess_response_signal(dest, IC_DBUS_SIGNAL_OBSERVE, signum, value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);
}


OCStackApplicationResult icd_ioty_ocprocess_observe_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	int ret, res;
	OCStackResult result;
	GVariantBuilder *options;
	struct icd_observe_context *observe_ctx;
	icd_sig_ctx_s *sig_context = ctx;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		_observe_cb_response_error(sig_context->bus_name, sig_context->signum,
				IOTCON_ERROR_IOTIVITY);
		return OC_STACK_KEEP_TRANSACTION;
	}

	observe_ctx = calloc(1, sizeof(struct icd_observe_context));
	if (NULL == observe_ctx) {
		ERR("calloc() Fail(%d)", errno);
		_observe_cb_response_error(sig_context->bus_name, sig_context->signum,
				IOTCON_ERROR_OUT_OF_MEMORY);
		return OC_STACK_KEEP_TRANSACTION;
	}

	result = resp->result;
	if (OC_STACK_OK == result) {
		res = IOTCON_RESPONSE_RESULT_OK;
		options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
				resp->numRcvdVendorSpecificHeaderOptions);
	} else {
		WARN("resp error(%d)", result);
		res = IOTCON_RESPONSE_RESULT_ERROR;
		options = NULL;
	}

	observe_ctx->payload = icd_payload_to_gvariant(resp->payload);
	observe_ctx->signum = sig_context->signum;
	observe_ctx->res = res;
	observe_ctx->bus_name = ic_utils_strdup(sig_context->bus_name);
	observe_ctx->options = options;

	ret = _ocprocess_worker_start(_worker_observe_cb, observe_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		_observe_cb_response_error(sig_context->bus_name, sig_context->signum, ret);
		free(observe_ctx->bus_name);
		if (observe_ctx->payload)
			g_variant_unref(observe_ctx->payload);
		g_variant_builder_unref(observe_ctx->options);
		free(observe_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* DO NOT FREE sig_context. It MUST be freed in the ocstack */
	/* DO NOT FREE observe_ctx. It MUST be freed in the _worker_observe_cb func */

	return OC_STACK_KEEP_TRANSACTION;
}


static int _worker_presence_cb(void *context)
{
	FN_CALL;
	int ret;
	GVariant *value;
	char addr[PATH_MAX] = {0};
	struct icd_presence_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	snprintf(addr, sizeof(addr), "%s:%d", ctx->dev_addr->addr, ctx->dev_addr->port);

	value = g_variant_new("(ius)", ctx->result, ctx->nonce, addr);

	ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_PRESENCE, ctx->signum,
			value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	/* ctx was allocated from icd_ioty_ocprocess_presence_cb() */
	free(ctx->bus_name);
	free(ctx->dev_addr);
	free(ctx);

	return ret;
}


static void _presence_cb_response_error(const char *dest, unsigned int signum,
		int ret_val)
{
	FN_CALL;
	int ret;
	GVariant *value;

	value = g_variant_new("(ius)", ret_val, 0, IC_STR_NULL);

	ret = _ocprocess_response_signal(dest, IC_DBUS_SIGNAL_PRESENCE, signum, value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);
}


OCStackApplicationResult icd_ioty_ocprocess_presence_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret;
	OCDevAddr *dev_addr;
	icd_sig_ctx_s *sig_context = ctx;
	struct icd_presence_context *presence_ctx;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);

	presence_ctx = calloc(1, sizeof(struct icd_presence_context));
	if (NULL == presence_ctx) {
		ERR("calloc() Fail(%d)", errno);
		_presence_cb_response_error(sig_context->bus_name, sig_context->signum,
				IOTCON_ERROR_OUT_OF_MEMORY);
		return OC_STACK_KEEP_TRANSACTION;
	}

	dev_addr = calloc(1, sizeof(OCDevAddr));
	if (NULL == dev_addr) {
		ERR("calloc() Fail(%d)", errno);
		_presence_cb_response_error(sig_context->bus_name, sig_context->signum,
				IOTCON_ERROR_OUT_OF_MEMORY);
		free(presence_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}
	memcpy(dev_addr, &resp->devAddr, sizeof(OCDevAddr));

	switch (resp->result) {
	case OC_STACK_OK:
		presence_ctx->result = IOTCON_PRESENCE_OK;
		break;
	case OC_STACK_PRESENCE_STOPPED:
		presence_ctx->result = IOTCON_PRESENCE_STOPPED;
		break;
	case OC_STACK_PRESENCE_TIMEOUT:
		presence_ctx->result = IOTCON_PRESENCE_TIMEOUT;
		break;
	case OC_STACK_ERROR:
	default:
		DBG("Presence error(%d)", resp->result);
		presence_ctx->result = IOTCON_ERROR_IOTIVITY;
	}

	presence_ctx->signum = sig_context->signum;
	presence_ctx->bus_name = ic_utils_strdup(sig_context->bus_name);
	presence_ctx->nonce = resp->sequenceNumber;
	presence_ctx->dev_addr = dev_addr;

	ret = _ocprocess_worker_start(_worker_presence_cb, presence_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		_presence_cb_response_error(sig_context->bus_name, sig_context->signum, ret);
		free(presence_ctx->bus_name);
		free(presence_ctx->dev_addr);
		free(presence_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* DO NOT FREE sig_context. It MUST be freed in the ocstack */
	/* DO NOT FREE presence_ctx. It MUST be freed in the _worker_presence_cb func */

	return OC_STACK_KEEP_TRANSACTION;
}


OCStackApplicationResult icd_ioty_ocprocess_info_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	int ret;
	int info_type;
	struct icd_info_context *info_ctx;
	icd_sig_ctx_s *sig_context = ctx;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp->payload, OC_STACK_KEEP_TRANSACTION);

	if (PAYLOAD_TYPE_DEVICE == resp->payload->type)
		info_type = ICD_DEVICE_INFO;
	else if (PAYLOAD_TYPE_PLATFORM == resp->payload->type)
		info_type = ICD_PLATFORM_INFO;
	else
		return OC_STACK_KEEP_TRANSACTION;

	info_ctx = calloc(1, sizeof(struct icd_info_context));
	if (NULL == info_ctx) {
		ERR("calloc() Fail(%d)", errno);
		return OC_STACK_KEEP_TRANSACTION;
	}

	info_ctx->info_type = info_type;
	info_ctx->payload = icd_payload_to_gvariant(resp->payload);
	info_ctx->signum = sig_context->signum;
	info_ctx->bus_name = ic_utils_strdup(sig_context->bus_name);

	ret = _ocprocess_worker_start(_worker_info_cb, info_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		free(info_ctx->bus_name);
		if (info_ctx->payload)
			g_variant_unref(info_ctx->payload);
		free(info_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* DO NOT FREE sig_context. It MUST be freed in the ocstack */
	/* DO NOT FREE info_ctx. It MUST be freed in the _worker_info_cb func */

	return OC_STACK_KEEP_TRANSACTION;
}


