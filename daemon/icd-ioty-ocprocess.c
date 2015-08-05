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
#include <json-glib/json-glib.h>

#include <ocstack.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icd.h"
#include "icd-dbus.h"
#include "icd-ioty.h"
#include "icd-ioty-ocprocess.h"

static int icd_ioty_alive;

typedef int (*_ocprocess_fn)(void *user_data);

struct icd_ioty_worker
{
	void *ctx;
	_ocprocess_fn fn;
};


enum _icd_secure_type
{
	ICD_TRANSPORT_IPV4_SECURE,
	ICD_TRANSPORT_IPV4
};


struct icd_req_context {
	unsigned int signum;
	char *bus_name;
	char *payload;
	int types;
	int observer_id;
	int observe_action;
	OCRequestHandle request_h;
	OCResourceHandle resource_h;
	GVariantBuilder *options;
	GVariantBuilder *query;
};


struct icd_find_context {
	unsigned int signum;
	char *bus_name;
	char *payload;
	OCDevAddr *dev_addr;
	int conn_type;
};


struct icd_crud_context {
	int res;
	int crud_type;
	char *payload;
	GVariantBuilder *options;
	GDBusMethodInvocation *invocation;
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

	ret = worker->fn(worker->ctx);
	if (IOTCON_ERROR_NONE != ret)
		ERR("fn() Fail(%d)", ret);

	/* worker was allocated from _ocprocess_worker_start() */
	free(worker);

	/* GCC warning happen if use g_thread_exit() */
	return NULL;
}


static int _ocprocess_worker_start(_ocprocess_fn fn, void *ctx)
{
	GError *error;
	GThread *thread;
	struct icd_ioty_worker *worker;

	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	worker = calloc(1, sizeof(struct icd_ioty_worker));
	if (NULL == worker) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	worker->fn = fn;
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
		return IOTCON_ERROR_UNKNOWN;
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

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_variant_new("(ia(qs)a(ss)iisii)",
			ctx->types,
			ctx->options,
			ctx->query,
			ctx->observe_action,
			ctx->observer_id,
			ctx->payload,
			GPOINTER_TO_INT(ctx->request_h),
			GPOINTER_TO_INT(ctx->resource_h));

	ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_REQUEST_HANDLER,
			ctx->signum, value);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ocprocess_response_signal() Fail(%d)", ret);

	free(ctx->bus_name);
	free(ctx->payload);
	g_variant_builder_unref(ctx->options);
	g_variant_builder_unref(ctx->query);
	free(ctx);

	return ret;
}


OCEntityHandlerResult icd_ioty_ocprocess_req_handler(OCEntityHandlerFlag flag,
		OCEntityHandlerRequest *request)
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
			req_ctx->payload = strdup(IC_STR_NULL);

			if (OC_OBSERVE_FLAG & flag) {
				req_ctx->types |= IOTCON_REQUEST_OBSERVE;
				/* observation info*/
				req_ctx->observer_id = request->obsInfo.obsId;
				req_ctx->observe_action = request->obsInfo.action;
			}
			break;
		case OC_REST_PUT:
			req_ctx->types = IOTCON_REQUEST_PUT;
			req_ctx->payload = ic_utils_strdup(request->reqJSONPayload);
			break;
		case OC_REST_POST:
			req_ctx->types = IOTCON_REQUEST_POST;
			req_ctx->payload = ic_utils_strdup(request->reqJSONPayload);
			break;
		case OC_REST_DELETE:
			req_ctx->types = IOTCON_REQUEST_DELETE;
			req_ctx->payload = strdup(IC_STR_NULL);
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
	while ((token = strtok_r(query_str, "&", &save_ptr1))) {
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
		free(req_ctx->payload);
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


/*
 * returned string SHOULD be released by you
 */
static inline char* _find_cb_get_address(OCDevAddr *address, int sec_type, int sec_port)
{
	FN_CALL;
	int ret;
	uint16_t port;
	uint8_t a, b, c, d;
	char addr[1024] = {0};

	RETVM_IF(ICD_TRANSPORT_IPV4 != sec_type && ICD_TRANSPORT_IPV4_SECURE != sec_type,
			NULL, "Invalid secure type(%d)", sec_type);

	ret = OCDevAddrToIPv4Addr(address, &a, &b, &c, &d);
	if (OC_STACK_OK != ret) {
		ERR("OCDevAddrToIPv4Addr() Fail(%d)", ret);
		return NULL;
	}

	if (ICD_TRANSPORT_IPV4_SECURE == sec_type) {
		if (sec_port <= 0 || 65535 < sec_port) {
			SECURE_ERR("Invalid secure port(%d)", sec_port);
			return NULL;
		}

		ret = snprintf(addr, sizeof(addr), ICD_IOTY_COAPS"%d.%d.%d.%d:%d", a, b, c, d,
				sec_port);
	} else {
		ret = OCDevAddrToPort(address, &port);
		if (OC_STACK_OK != ret) {
			ERR("OCDevAddrToPort() Fail(%d)", ret);
			return NULL;
		}

		ret = snprintf(addr, sizeof(addr), ICD_IOTY_COAP"%d.%d.%d.%d:%d", a, b, c, d,
				port);
	}

	WARN_IF(ret <= 0 || sizeof(addr) <= ret, "snprintf() Fail(%d)", ret);

	return ic_utils_strdup(addr);
}


static inline int _find_cb_response(JsonObject *rsrc_obj,
		struct icd_find_context *ctx)
{
	GVariant *value;
	JsonGenerator *gen;
	JsonNode *root_node;
	char *host, *json_data;
	JsonObject *property_obj;
	int ret, secure, secure_type, secure_port;

	RETV_IF(NULL == rsrc_obj, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	/* parse secure secure_port */
	property_obj = json_object_get_object_member(rsrc_obj, IC_JSON_KEY_PROPERTY);
	if (NULL == property_obj) {
		ERR("json_object_get_object_member() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	secure = json_object_get_int_member(property_obj, IC_JSON_KEY_SECURE);
	if (0 == secure) {
		secure_type = ICD_TRANSPORT_IPV4;
		secure_port = 0;
	} else {
		secure_type = ICD_TRANSPORT_IPV4_SECURE;
		secure_port = json_object_get_int_member(property_obj, IC_JSON_KEY_PORT);
	}

	host = _find_cb_get_address(ctx->dev_addr, secure_type, secure_port);
	if (NULL == host) {
		ERR("_find_cb_get_address() Fail");
		return IOTCON_ERROR_IOTIVITY;
	}

	gen = json_generator_new();
	root_node = json_node_new(JSON_NODE_OBJECT);
	json_node_set_object(root_node, rsrc_obj);
	json_generator_set_root(gen, root_node);

	json_data = json_generator_to_data(gen, NULL);
	json_node_free(root_node);
	g_object_unref(gen);

	value = g_variant_new("(ssi)", json_data, host, ctx->conn_type);
	free(json_data);
	free(host);

	/* TODO : If one device has multi resources, it comes as bulk data.
	 * To reduce the number of emit_signal, let's send signal only one time for one device.
	 * for ex, client list. */
	ret = _ocprocess_response_signal(ctx->bus_name, IC_DBUS_SIGNAL_FOUND_RESOURCE,
			ctx->signum, value);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_response_signal() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


static inline int _find_cb_handle_context(struct icd_find_context *ctx)
{
	int ret;
	JsonParser *parser;
	GError *error = NULL;
	JsonObject *root_obj;
	JsonArray *rsrc_array;
	unsigned int rsrc_count, rsrc_index;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ctx->payload, IOTCON_ERROR_INVALID_PARAMETER);

	parser = json_parser_new();
	ret = json_parser_load_from_data(parser, ctx->payload, strlen(ctx->payload), &error);
	if (FALSE == ret) {
		ERR("json_parser_load_from_data() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	/* parse 'oc' prefix */
	root_obj = json_node_get_object(json_parser_get_root(parser));
	rsrc_array = json_object_get_array_member(root_obj, IC_JSON_KEY_OC);
	if (NULL == rsrc_array) {
		ERR("json_object_get_array_member() Fail");
		g_object_unref(parser);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	rsrc_count = json_array_get_length(rsrc_array);
	if (0 == rsrc_count) {
		ERR("Invalid count(%d)", rsrc_count);
		g_object_unref(parser);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	for (rsrc_index = 0; rsrc_index < rsrc_count; rsrc_index++) {
		JsonObject *rsrc_obj;

		rsrc_obj = json_array_get_object_element(rsrc_array, rsrc_index);
		if (0 == json_object_get_size(rsrc_obj)) /* for the case of empty "{}" */
			continue;

		ret = _find_cb_response(rsrc_obj, ctx);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_find_cb_response() Fail(%d)", ret);
			g_object_unref(parser);
			return ret;
		}
	}

	g_object_unref(parser);

	return IOTCON_ERROR_NONE;
}


static int _worker_find_cb(void *context)
{
	int ret;
	struct icd_find_context *ctx = context;

	RETV_IF(NULL == ctx, IOTCON_ERROR_INVALID_PARAMETER);

	ret = _find_cb_handle_context(ctx);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_find_cb_handle_context() Fail(%d)", ret);

	/* ctx was allocated from icd_ioty_ocprocess_find_cb() */
	free(ctx->bus_name);
	free(ctx->payload);
	free(ctx->dev_addr);
	free(ctx);

	return ret;
}


OCStackApplicationResult icd_ioty_ocprocess_find_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	int ret;
	OCDevAddr *dev_addr;
	struct icd_find_context *find_ctx;
	icd_sig_ctx_s *sig_context = ctx;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp->resJSONPayload, OC_STACK_KEEP_TRANSACTION);

	find_ctx = calloc(1, sizeof(struct icd_find_context));
	if (NULL == find_ctx) {
		ERR("calloc() Fail(%d)", errno);
		return OC_STACK_KEEP_TRANSACTION;
	}

	dev_addr = calloc(1, sizeof(OCDevAddr));
	if (NULL == dev_addr) {
		ERR("calloc() Fail(%d)", errno);
		free(find_ctx);
		return OC_STACK_KEEP_TRANSACTION;
	}
	memcpy(dev_addr, resp->addr, sizeof(OCDevAddr));

	find_ctx->signum = sig_context->signum;
	find_ctx->bus_name = ic_utils_strdup(sig_context->bus_name);
	find_ctx->payload = ic_utils_strdup(resp->resJSONPayload);
	find_ctx->dev_addr = dev_addr;
	find_ctx->conn_type = resp->connType;

	ret = _ocprocess_worker_start(_worker_find_cb, find_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		free(find_ctx->bus_name);
		free(find_ctx->payload);
		free(find_ctx->dev_addr);
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
		value = g_variant_new("(a(qs)si)", ctx->options, ctx->payload, ctx->res);
	icd_ioty_complete(ctx->crud_type, ctx->invocation, value);

	/* ctx was allocated from icd_ioty_ocprocess_xxx_cb() */
	free(ctx->payload);
	g_variant_builder_unref(ctx->options);
	free(ctx);

	return IOTCON_ERROR_NONE;
}


static int _ocprocess_worker(_ocprocess_fn fn, int type, const char *payload, int res,
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
	crud_ctx->payload = strdup(ic_utils_dbus_encode_str(payload));
	crud_ctx->res = res;
	crud_ctx->options = options;
	crud_ctx->invocation = ctx;

	ret = _ocprocess_worker_start(fn, crud_ctx);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ocprocess_worker_start() Fail(%d)", ret);
		free(crud_ctx->payload);
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
	struct icd_crud_context *crud_ctx;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->resJSONPayload || '\0' == resp->resJSONPayload[0]) {
		ERR("json payload is empty");
		icd_ioty_complete_error(ICD_CRUD_GET, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	crud_ctx = calloc(1, sizeof(struct icd_crud_context));
	if (NULL == crud_ctx) {
		ERR("calloc() Fail(%d)", errno);
		icd_ioty_complete_error(ICD_CRUD_GET, ctx, IOTCON_ERROR_OUT_OF_MEMORY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	result = resp->result;
	if (result == OC_STACK_OK) {
		res = IOTCON_RESPONSE_RESULT_OK;
		options = _ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
				resp->numRcvdVendorSpecificHeaderOptions);
	} else {
		WARN("resp error(%d)", result);
		res = IOTCON_RESPONSE_RESULT_ERROR;
		options = NULL;
	}

	ret = _ocprocess_worker(_worker_crud_cb, ICD_CRUD_GET, resp->resJSONPayload, res,
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

	if (NULL == resp->resJSONPayload || '\0' == resp->resJSONPayload[0]) {
		ERR("json payload is empty");
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

	ret = _ocprocess_worker(_worker_crud_cb, ICD_CRUD_PUT, resp->resJSONPayload, res,
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
	struct icd_crud_context *crud_ctx;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->resJSONPayload || '\0' == resp->resJSONPayload[0]) {
		ERR("json payload is empty");
		icd_ioty_complete_error(ICD_CRUD_POST, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	crud_ctx = calloc(1, sizeof(struct icd_crud_context));
	if (NULL == crud_ctx) {
		ERR("calloc() Fail(%d)", errno);
		icd_ioty_complete_error(ICD_CRUD_POST, ctx, IOTCON_ERROR_OUT_OF_MEMORY);
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

	ret = _ocprocess_worker(_worker_crud_cb, ICD_CRUD_POST, resp->resJSONPayload, res,
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
	struct icd_crud_context *crud_ctx;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);

	if (NULL == resp->resJSONPayload || '\0' == resp->resJSONPayload[0]) {
		ERR("json payload is empty");
		icd_ioty_complete_error(ICD_CRUD_DELETE, ctx, IOTCON_ERROR_IOTIVITY);
		return OC_STACK_DELETE_TRANSACTION;
	}

	crud_ctx = calloc(1, sizeof(struct icd_crud_context));
	if (NULL == crud_ctx) {
		ERR("calloc() Fail(%d)", errno);
		icd_ioty_complete_error(ICD_CRUD_DELETE, ctx, IOTCON_ERROR_OUT_OF_MEMORY);
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

	/* DO NOT FREE crud_ctx. It MUST be freed in the _worker_delete_cb func */

	return OC_STACK_DELETE_TRANSACTION;
}

