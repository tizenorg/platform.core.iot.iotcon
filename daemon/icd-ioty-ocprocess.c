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

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
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
	int64_t signal_number;
	char *bus_name;
	int request_type;
	int observe_id;
	int observe_type;
	OCRequestHandle request_h;
	OCResourceHandle resource_h;
	GVariant *payload;
	GVariantBuilder *options;
	GVariantBuilder *query;
	OCDevAddr *dev_addr;
};


struct icd_find_context {
	int64_t signal_number;
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
	int64_t signal_number;
	int info_type;
	char *bus_name;
	GVariant *payload;
};


struct icd_tizen_info_context {
	OCRequestHandle request_h;
	OCResourceHandle resource_h;
	GDBusMethodInvocation *invocation;
};


struct icd_observe_context {
	int64_t signal_number;
	int res;
	int seqnum;
	char *bus_name;
	GVariant *payload;
	GVariantBuilder *options;
};


struct icd_presence_context {
	OCDoHandle handle;
	int result;
	unsigned int nonce;
	OCDevAddr *dev_addr;
	iotcon_presence_trigger_e trigger;
	char *resource_type;
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


static int _ocprocess_response_signal(const char *dest, const char *signal_prefix,
		int64_t signal_number, GVariant *value)
{
	int ret;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	ret = snprintf(signal_name, sizeof(signal_name), "%s_%llx", signal_prefix, signal_number);
	if (ret <= 0 || sizeof(signal_name) <= ret) {
		ERR("snprintf() Fail(%d)", ret);
		return IOTCON_ERROR_IO_ERROR;
	}

	ret = icd_dbus_emit_signal(dest, signal_name, value);
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

static int _ioty_oic_action_to_ioty_action(int oic_action)
{
	int action;

	switch (oic_action) {
	case OC_OBSERVE_REGISTER:
		action = IOTCON_OBSERVE_REGISTER;
		break;
	case OC_OBSERVE_DEREGISTER:
		action = IOTCON_OBSERVE_DEREGISTER;
		break;
	case OC_OBSERVE_NO_OPTION:
	default:
		ERR("Invalid action (%d)", oic_action);
		action = IOTCON_OBSERVE_NO_TYPE;
	}
	return action;
}


static void _icd_req_context_free(struct icd_req_context *ctx)
{
	free(ctx->bus_name);
	if (ctx->payload)
		g_variant_unref(ctx->payload);
	g_variant_builder_unref(ctx->options);
	g_variant_builder_unref(ctx->query);
	free(ctx->dev_addr);
	free(ctx);
}


static int _worker_req_handler(void *context)
{
	GVariant *value;
	char *host_address;
	int ret, conn_type;
	GVariantBuilder payload_builder;
	struct icd_req_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	g_variant_builder_init(&payload_builder, G_VARIANT_TYPE("av"));
	if (ctx->payload) {
		g_variant_builder_add(&payload_builder, "v", ctx->payload);
		ctx->payload = NULL;
	}

	ret = icd_ioty_get_host_address(ctx->dev_addr, &host_address, &conn_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_get_host_address() Fail(%d)", ret);
		g_variant_builder_clear(&payload_builder);
		_icd_req_context_free(ctx);
		return ret;
	}

	value = g_variant_new("(siia(qs)a(ss)iiavxx)",
			host_address,
			conn_type,
			ctx->request_type,
			ctx->options,
			ctx->query,
			ctx->observe_type,
			ctx->observe_id,
			&payload_builder,
			ICD_POINTER_TO_INT64(ctx->request_h),
			ICD_POINTER_TO_INT64(ctx->resource_h));

	free(host_address);

	ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_REQUEST_HANDLER,
			ctx->signal_number, value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	_icd_req_context_free(ctx);

	return ret;
}


OCEntityHandlerResult icd_ioty_ocprocess_req_handler(OCEntityHandlerFlag flag,
		OCEntityHandlerRequest *request, void *user_data)
{
	FN_CALL;
	int ret;
	int64_t signal_number;
	char *query_str, *query_key, *query_value;
	char *token, *save_ptr1, *save_ptr2;
	char *bus_name = NULL;
	struct icd_req_context *req_ctx;
	OCDevAddr *dev_addr;

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
	req_ctx->signal_number = signal_number;
	req_ctx->bus_name = bus_name;

	dev_addr = calloc(1, sizeof(OCDevAddr));
	if (NULL == dev_addr) {
		ERR("calloc() Fail(%d)", errno);
		free(req_ctx->bus_name);
		free(req_ctx);
		return OC_EH_ERROR;
	}
	memcpy(dev_addr, &request->devAddr, sizeof(OCDevAddr));
	req_ctx->dev_addr = dev_addr;

	/* request type */
	if (OC_REQUEST_FLAG & flag) {
		switch (request->method) {
		case OC_REST_GET:
			req_ctx->request_type = IOTCON_REQUEST_GET;
			req_ctx->payload = NULL;
			break;
		case OC_REST_PUT:
			req_ctx->request_type = IOTCON_REQUEST_PUT;
			req_ctx->payload = icd_payload_to_gvariant(request->payload);
			break;
		case OC_REST_POST:
			req_ctx->request_type = IOTCON_REQUEST_POST;
			req_ctx->payload = icd_payload_to_gvariant(request->payload);
			break;
		case OC_REST_DELETE:
			req_ctx->request_type = IOTCON_REQUEST_DELETE;
			req_ctx->payload = NULL;
			break;
		default:
			free(req_ctx->dev_addr);
			free(req_ctx->bus_name);
			free(req_ctx);
			return OC_EH_ERROR;
		}
	}

	if (OC_OBSERVE_FLAG & flag) {
		/* observation info*/
		req_ctx->observe_id = request->obsInfo.obsId;
		req_ctx->observe_type = _ioty_oic_action_to_ioty_action(request->obsInfo.action);
	} else {
		req_ctx->observe_type = IOTCON_OBSERVE_NO_TYPE;
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
		_icd_req_context_free(req_ctx);
		return OC_EH_ERROR;
	}

	/* DO NOT FREE req_ctx. It MUST be freed in the _worker_req_handler func */

	return OC_EH_OK;
}


gpointer icd_ioty_ocprocess_thread(gpointer data)
{
	FN_CALL;
	OCStackResult result;
	const struct timespec delay = {0, 10 * 1000 * 1000}; /* 10 ms */

	icd_ioty_alive = 1;
	while (icd_ioty_alive) {
		icd_ioty_csdk_lock();
		result = OCProcess();
		icd_ioty_csdk_unlock();
		if (OC_STACK_OK != result) {
			ERR("OCProcess() Fail(%d)", result);
			break;
		}

		/* TODO : Current '10ms' is not proven sleep time. Revise the time after test.
		 * Or recommend changes to event driven architecture */
		nanosleep(&delay, NULL);
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
				ctx->signal_number, value);
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

	find_ctx->signal_number = sig_context->signal_number;
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
	const char *signal_prefix = NULL;
	struct icd_info_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	if (ICD_DEVICE_INFO == ctx->info_type)
		signal_prefix = IC_DBUS_SIGNAL_DEVICE;
	else if (ICD_PLATFORM_INFO == ctx->info_type)
		signal_prefix = IC_DBUS_SIGNAL_PLATFORM;

	ret = _ocprocess_response_signal(ctx->bus_name, signal_prefix, ctx->signal_number,
			ctx->payload);
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

static int _ocprocess_parse_oic_result(OCStackResult result)
{
	int res;

	switch (result) {
	case OC_STACK_OK:
		res = IOTCON_RESPONSE_OK;
		break;
	case OC_STACK_RESOURCE_CREATED:
		res = IOTCON_RESPONSE_RESOURCE_CREATED;
		break;
	case OC_STACK_RESOURCE_DELETED:
		res = IOTCON_RESPONSE_RESULT_DELETED;
		break;
	case OC_STACK_UNAUTHORIZED_REQ:
		res = IOTCON_RESPONSE_FORBIDDEN;
		break;
	default:
		WARN("response error(%d)", result);
		res = IOTCON_RESPONSE_ERROR;
		break;
	}

	return res;
}


OCStackApplicationResult icd_ioty_ocprocess_get_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret, res;
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_GET, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	res = _ocprocess_parse_oic_result(resp->result);

	options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions);

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
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_PUT, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	res = _ocprocess_parse_oic_result(resp->result);

	options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions);

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
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_POST, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	res = _ocprocess_parse_oic_result(resp->result);

	options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions);

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
	GVariantBuilder *options;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_CRUD_DELETE, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	res = _ocprocess_parse_oic_result(resp->result);

	options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions);

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

	ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_OBSERVE,
			ctx->signal_number, value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	/* ctx was allocated from icd_ioty_ocprocess_observe_cb() */
	free(ctx->bus_name);
	g_variant_builder_unref(ctx->options);
	free(ctx);

	return ret;
}


static void _observe_cb_response_error(const char *dest,
		int64_t signal_number, int ret_val)
{
	int ret;
	GVariant *value;
	GVariant *payload;
	GVariantBuilder options;

	g_variant_builder_init(&options, G_VARIANT_TYPE("a(qs)"));
	payload = icd_payload_representation_empty_gvariant();

	value = g_variant_new("(a(qs)vii)", &options, payload, ret_val, 0);

	ret = _ocprocess_response_signal(dest, IC_DBUS_SIGNAL_OBSERVE, signal_number, value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);
}


OCStackApplicationResult icd_ioty_ocprocess_observe_cb(void *ctx,
		OCDoHandle handle, OCClientResponse *resp)
{
	int ret, res;
	GVariantBuilder *options;
	struct icd_observe_context *observe_ctx;
	icd_sig_ctx_s *sig_context = ctx;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		_observe_cb_response_error(sig_context->bus_name, sig_context->signal_number,
				IOTCON_ERROR_IOTIVITY);
		return OC_STACK_KEEP_TRANSACTION;
	}

	observe_ctx = calloc(1, sizeof(struct icd_observe_context));
	if (NULL == observe_ctx) {
		ERR("calloc() Fail(%d)", errno);
		_observe_cb_response_error(sig_context->bus_name, sig_context->signal_number,
				IOTCON_ERROR_OUT_OF_MEMORY);
		return OC_STACK_KEEP_TRANSACTION;
	}

	res = _ocprocess_parse_oic_result(resp->result);

	options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions);

	observe_ctx->payload = icd_payload_to_gvariant(resp->payload);
	observe_ctx->signal_number = sig_context->signal_number;
	observe_ctx->res = res;
	observe_ctx->bus_name = ic_utils_strdup(sig_context->bus_name);
	observe_ctx->options = options;

	ret = _ocprocess_worker_start(_worker_observe_cb, observe_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		_observe_cb_response_error(sig_context->bus_name, sig_context->signal_number, ret);
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
	int conn_type;
	OCDoHandle handle;
	char *host_address;
	GVariant *value, *value2;
	int ret = IOTCON_ERROR_NONE;
	struct icd_presence_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icd_ioty_get_host_address(ctx->dev_addr, &host_address, &conn_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_get_host_address() Fail(%d)", ret);
		free(ctx->resource_type);
		free(ctx->dev_addr);
		free(ctx);
		return ret;
	}

	value = g_variant_new("(iusiis)", ctx->result, ctx->nonce, host_address, conn_type,
			ctx->trigger, ic_utils_dbus_encode_str(ctx->resource_type));
	value2 = g_variant_ref(value);

	free(host_address);

	ret = _ocprocess_response_signal(NULL, IC_DBUS_SIGNAL_PRESENCE,
			ICD_POINTER_TO_INT64(ctx->handle), value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	handle = icd_ioty_presence_table_get_handle(ICD_MULTICAST_ADDRESS);
	if (handle && (handle != ctx->handle)) {
		ret = _ocprocess_response_signal(NULL, IC_DBUS_SIGNAL_PRESENCE,
				ICD_POINTER_TO_INT64(handle), value2);
		if (IOTCON_ERROR_NONE != ret)
			ERR("_ocprocess_response_signal() Fail(%d)", ret);
	} else {
		g_variant_unref(value2);
	}

	/* ctx was allocated from icd_ioty_ocprocess_presence_cb() */
	free(ctx->resource_type);
	free(ctx->dev_addr);
	free(ctx);

	return ret;
}


static void _presence_cb_response_error(OCDoHandle handle, int ret_val)
{
	FN_CALL;
	int ret;
	OCDoHandle handle2;
	GVariant *value, *value2;

	value = g_variant_new("(iusiis)", ret_val, 0, IC_STR_NULL, IOTCON_CONNECTIVITY_ALL,
			IOTCON_PRESENCE_RESOURCE_CREATED, IC_STR_NULL);
	value2 = g_variant_ref(value);

	ret = _ocprocess_response_signal(NULL, IC_DBUS_SIGNAL_PRESENCE,
			ICD_POINTER_TO_INT64(handle), value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	handle2 = icd_ioty_presence_table_get_handle(ICD_MULTICAST_ADDRESS);
	if (handle2 && (handle2 != handle)) {
		ret = _ocprocess_response_signal(NULL, IC_DBUS_SIGNAL_PRESENCE,
				ICD_POINTER_TO_INT64(handle), value2);
		if (IOTCON_ERROR_NONE != ret)
			ERR("_ocprocess_response_signal() Fail(%d)", ret);
	} else {
		g_variant_unref(value2);
	}
}


static int _presence_trigger_to_ioty_trigger(OCPresenceTrigger src,
		iotcon_presence_trigger_e *dest)
{
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	switch (src) {
	case OC_PRESENCE_TRIGGER_CREATE:
		*dest = IOTCON_PRESENCE_RESOURCE_CREATED;
		break;
	case OC_PRESENCE_TRIGGER_CHANGE:
		*dest = IOTCON_PRESENCE_RESOURCE_UPDATED;
		break;
	case OC_PRESENCE_TRIGGER_DELETE:
		*dest = IOTCON_PRESENCE_RESOURCE_DESTROYED;
		break;
	default:
		ERR("Invalid trigger(%d)", src);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


OCStackApplicationResult icd_ioty_ocprocess_presence_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret;
	OCDevAddr *dev_addr;
	OCPresencePayload *payload;
	struct icd_presence_context *presence_ctx;

	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);

	presence_ctx = calloc(1, sizeof(struct icd_presence_context));
	if (NULL == presence_ctx) {
		ERR("calloc() Fail(%d)", errno);
		_presence_cb_response_error(handle, IOTCON_ERROR_OUT_OF_MEMORY);
		return OC_STACK_KEEP_TRANSACTION;
	}

	dev_addr = calloc(1, sizeof(OCDevAddr));
	if (NULL == dev_addr) {
		ERR("calloc() Fail(%d)", errno);
		_presence_cb_response_error(handle, IOTCON_ERROR_OUT_OF_MEMORY);
		free(presence_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}
	memcpy(dev_addr, &resp->devAddr, sizeof(OCDevAddr));

	payload = (OCPresencePayload*)resp->payload;

	switch (resp->result) {
	case OC_STACK_OK:
		presence_ctx->result = IOTCON_PRESENCE_OK;
		ret = _presence_trigger_to_ioty_trigger(payload->trigger, &presence_ctx->trigger);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_presence_trigger_to_ioty_trigger() Fail(%d)", ret);
			_presence_cb_response_error(handle, ret);
			free(presence_ctx);
			return OC_STACK_KEEP_TRANSACTION;
		}
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

	presence_ctx->handle = handle;
	presence_ctx->nonce = resp->sequenceNumber;
	presence_ctx->dev_addr = dev_addr;

	if (payload->resourceType)
		presence_ctx->resource_type = strdup(payload->resourceType);

	ret = _ocprocess_worker_start(_worker_presence_cb, presence_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		_presence_cb_response_error(handle, ret);
		free(presence_ctx->resource_type);
		free(presence_ctx->dev_addr);
		free(presence_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}

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
	info_ctx->signal_number = sig_context->signal_number;
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


static int _worker_tizen_info_handler(void *context)
{
	int ret;
	char *device_name;
	char *tizen_device_id;
	OCStackResult result;
	OCRepPayload *payload;
	OCEntityHandlerResponse response = {0};
	struct icd_tizen_info_context *ctx = context;

	response.requestHandle = ctx->request_h;
	response.resourceHandle = ctx->resource_h;
	response.ehResult = OC_EH_OK;

	/* Get Tizen Info */
	ret = icd_ioty_tizen_info_get_property(&device_name, &tizen_device_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_tizen_info_get_property() Fail(%d)", ret);
		response.ehResult = OC_EH_ERROR;
	}

	/* payload */
	payload = OCRepPayloadCreate();
	OCRepPayloadSetUri(payload, ICD_IOTY_TIZEN_INFO_URI);
	OCRepPayloadAddResourceType(payload, ICD_IOTY_TIZEN_INFO_TYPE);
	OCRepPayloadAddInterface(payload, IC_INTERFACE_DEFAULT);

	OCRepPayloadSetPropString(payload, ICD_IOTY_TIZEN_INFO_DEVICE_NAME,
			ic_utils_dbus_encode_str(device_name));
	OCRepPayloadSetPropString(payload, ICD_IOTY_TIZEN_INFO_TIZEN_DEVICE_ID,
			ic_utils_dbus_encode_str(tizen_device_id));
	response.payload = (OCPayload*)payload;

	icd_ioty_csdk_lock();
	result = OCDoResponse(&response);
	icd_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCDoResponse() Fail(%d)", result);
		free(ctx);
		return IOTCON_ERROR_IOTIVITY;
	}

	free(ctx);
	return IOTCON_ERROR_NONE;
}


OCEntityHandlerResult icd_ioty_ocprocess_tizen_info_handler(OCEntityHandlerFlag flag,
		OCEntityHandlerRequest *request, void *user_data)
{
	int ret;
	struct icd_tizen_info_context *tizen_info_ctx;

	if ((0 == (OC_REQUEST_FLAG & flag)) || (OC_REST_GET != request->method)) {
		ERR("Prohibited Action");
		return OC_EH_FORBIDDEN;
	}

	tizen_info_ctx = calloc(1, sizeof(struct icd_tizen_info_context));
	if (NULL == tizen_info_ctx) {
		ERR("calloc() Fail(%d)", errno);
		return OC_EH_ERROR;
	}

	tizen_info_ctx->request_h = request->requestHandle;
	tizen_info_ctx->resource_h = request->resource;

	ret = _ocprocess_worker_start(_worker_tizen_info_handler, tizen_info_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		free(tizen_info_ctx);
		return OC_EH_ERROR;
	}

	return OC_EH_OK;
}


OCStackApplicationResult icd_ioty_ocprocess_get_tizen_info_cb(void *ctx,
		OCDoHandle handle, OCClientResponse *resp)
{
	int res;
	char *device_name;
	char *tizen_device_id;
	GVariant *tizen_info;
	OCRepPayload *payload;
	OCRepPayloadValue *val;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->payload) {
		ERR("payload is empty");
		icd_ioty_complete_error(ICD_TIZEN_INFO, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	payload = (OCRepPayload*)resp->payload;
	val = payload->values;
	if (NULL == val) {
		ERR("Invalid payload");
		icd_ioty_complete_error(ICD_TIZEN_INFO, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}
	device_name = val->str;

	val = val->next;
	if (NULL == val) {
		ERR("Invalid Payload");
		icd_ioty_complete_error(ICD_TIZEN_INFO, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	tizen_device_id = val->str;

	if (OC_STACK_OK == resp->result)
		res = IOTCON_RESPONSE_OK;
	else
		res = IOTCON_RESPONSE_ERROR;

	tizen_info = g_variant_new("(ssi)", device_name, tizen_device_id, res);

	icd_ioty_complete(ICD_TIZEN_INFO, ctx, tizen_info);

	return OC_STACK_DELETE_TRANSACTION;
}
