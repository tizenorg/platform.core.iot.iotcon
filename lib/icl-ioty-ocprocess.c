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
#include <ocstack.h>
#include <octypes.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "ic-ioty-types.h"
#include "icl.h"
#include "icl-ioty.h"
#include "icl-remote-resource.h"
#include "icl-device.h"
#include "icl-presence.h"
#include "icl-request.h"
#include "icl-resource.h"
#include "icl-response.h"
#include "icl-options.h"
#include "icl-representation.h"
#include "icl-types.h"
#include "icl-state.h"
#include "icl-lite-resource.h"
#include "icl-ioty.h"
#include "icl-ioty-types.h"
#include "icl-ioty-ocprocess.h"

static int icl_ioty_alive = 1;

void icl_ioty_ocprocess_stop()
{
	icl_ioty_alive = 0;
}

void icl_ioty_ocprocess_start()
{
	icl_ioty_alive = 1;
}

gpointer icl_ioty_ocprocess_thread(gpointer data)
{
	FN_CALL;
	OCStackResult result;
	const struct timespec delay = {0, 10 * 1000 * 1000}; /* 10 ms */

	while (icl_ioty_alive) {
		icl_ioty_csdk_lock();
		result = OCProcess();
		icl_ioty_csdk_unlock();
		if (OC_STACK_OK != result) {
			ERR("OCProcess() Fail(%d)", result);
			break;
		}

		// TODO: Current '10ms' is not proven sleep time. Revise the time after test.
		// TODO: Or recommend changes to event driven architecture
		nanosleep(&delay, NULL);
	}

	return NULL;
}


static gboolean _icl_ioty_ocprocess_find_idle_cb(gpointer p)
{
	int i;
	icl_find_cb_s *cb_data = p;

	RETV_IF(NULL == cb_data, G_SOURCE_REMOVE);

	if (cb_data->cb) {
		for (i = 0; i < cb_data->resource_count; i++) {
			cb_data->resource_list[i]->is_found = true;
			cb_data->cb(cb_data->resource_list[i], IOTCON_ERROR_NONE, cb_data->user_data);
		}
	}

	icl_destroy_find_cb_data(cb_data);

	return G_SOURCE_REMOVE;
}

OCStackApplicationResult icl_ioty_ocprocess_find_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret, resource_count;
	iotcon_remote_resource_h *resource_list;
	icl_find_cb_s *find_cb_data = NULL;
	icl_cb_s *cb_data = ctx;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	if (NULL == resp->payload) /* normal case : payload COULD be NULL */
		return OC_STACK_KEEP_TRANSACTION;
	RETVM_IF(PAYLOAD_TYPE_DISCOVERY != resp->payload->type,
			OC_STACK_KEEP_TRANSACTION, "Invalid payload type(%d)", resp->payload->type);

	ret = icl_ioty_parse_oic_discovery_payload(&(resp->devAddr),
			(OCDiscoveryPayload*)resp->payload, &resource_list, &resource_count);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_ocprocess_parse_find_payload() Fail(%d)", ret);
		if (cb_data->cb)
			((iotcon_found_resource_cb)cb_data->cb)(NULL, ret, cb_data->user_data);
		return OC_STACK_KEEP_TRANSACTION;
	}
	cb_data->found = true;

	ret = icl_create_find_cb_data(cb_data, resource_list, resource_count, &find_cb_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_create_find_cb_data() Fail(%d)", ret);
		if (cb_data->cb)
			((iotcon_found_resource_cb)cb_data->cb)(NULL, ret, cb_data->user_data);
		return OC_STACK_KEEP_TRANSACTION;
	}

	g_idle_add(_icl_ioty_ocprocess_find_idle_cb, find_cb_data);

	/* DO NOT FREE ctx(cb_data). It MUST be freed in the ocstack */

	return OC_STACK_KEEP_TRANSACTION;
}

static gboolean _icl_ioty_ocprocess_device_info_idle_cb(gpointer p)
{
	icl_device_cb_s *cb_data = p;

	RETV_IF(NULL == cb_data, G_SOURCE_REMOVE);

	if (cb_data->cb)
		cb_data->cb(cb_data->device_info, IOTCON_ERROR_NONE, cb_data->user_data);

	icl_destroy_device_cb_data(cb_data);

	return G_SOURCE_REMOVE;
}

OCStackApplicationResult icl_ioty_ocprocess_device_info_cb(void *ctx,
		OCDoHandle handle, OCClientResponse* resp)
{
	FN_CALL;
	int ret;
	icl_cb_s *cb_data = ctx;
	icl_device_cb_s *device_cb_data = NULL;
	iotcon_device_info_h info;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp->payload, OC_STACK_KEEP_TRANSACTION);

	if (PAYLOAD_TYPE_DEVICE != resp->payload->type) {
		ERR("Invalid payload type (%d)", resp->payload->type);
		return OC_STACK_KEEP_TRANSACTION;
	}

	ret = icl_ioty_parse_oic_device_payload((OCDevicePayload *)resp->payload,
			&info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_device_payload() Fail(%d)", ret);
		if (cb_data->cb)
			((iotcon_device_info_cb)cb_data->cb)(NULL, ret, cb_data->user_data);
		return OC_STACK_KEEP_TRANSACTION;
	}
	cb_data->found = true;

	ret = icl_create_device_cb_data(cb_data, info, &device_cb_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_create_device_cb_data() Fail(%d)", ret);
		if (cb_data->cb)
			((iotcon_device_info_cb)cb_data->cb)(NULL, ret, cb_data->user_data);
		return OC_STACK_KEEP_TRANSACTION;
	}

	g_idle_add(_icl_ioty_ocprocess_device_info_idle_cb, device_cb_data);

	/* DO NOT FREE ctx(cb_data). It MUST be freed in the ocstack */

	return OC_STACK_KEEP_TRANSACTION;
}

static gboolean _icl_ioty_ocprocess_platform_info_idle_cb(gpointer p)
{
	icl_platform_cb_s *cb_data = p;

	RETV_IF(NULL == cb_data, G_SOURCE_REMOVE);

	if (cb_data->cb)
		cb_data->cb(cb_data->platform_info, IOTCON_ERROR_NONE, cb_data->user_data);

	icl_destroy_platform_cb_data(cb_data);

	return G_SOURCE_REMOVE;
}

OCStackApplicationResult icl_ioty_ocprocess_platform_info_cb(void *ctx,
		OCDoHandle handle, OCClientResponse* resp)
{
	FN_CALL;
	int ret;
	icl_cb_s *cb_data = ctx;
	icl_platform_cb_s *platform_cb_data = NULL;
	iotcon_platform_info_h info;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp->payload, OC_STACK_KEEP_TRANSACTION);

	if (PAYLOAD_TYPE_PLATFORM != resp->payload->type) {
		ERR("Invalid payload type (%d)", resp->payload->type);
		return OC_STACK_KEEP_TRANSACTION;
	}

	ret = icl_ioty_parse_oic_platform_payload((OCPlatformPayload *)resp->payload,
			&info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_platform_payload() Fail(%d)", ret);
		if (cb_data->cb)
			((iotcon_platform_info_cb)cb_data->cb)(NULL, ret, cb_data->user_data);
		return OC_STACK_KEEP_TRANSACTION;
	}
	cb_data->found = true;

	ret = icl_create_platform_cb_data(cb_data, info, &platform_cb_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_create_platform_cb_data() Fail(%d)", ret);
		if (cb_data->cb)
			((iotcon_platform_info_cb)cb_data->cb)(NULL, ret, cb_data->user_data);
		return OC_STACK_KEEP_TRANSACTION;
	}

	g_idle_add(_icl_ioty_ocprocess_platform_info_idle_cb, platform_cb_data);

	/* DO NOT FREE ctx(cb_data). It MUST be freed in the ocstack */

	return OC_STACK_KEEP_TRANSACTION;
}

static gboolean _icl_ioty_ocprocess_presence_idle_cb(gpointer p)
{
	icl_presence_cb_s *cb_data = p;
	iotcon_presence_h presence;

	RETV_IF(NULL == cb_data, G_SOURCE_REMOVE);

	presence = cb_data->presence;
	if (NULL == presence) {
		ERR("presence is NULL");
		icl_destroy_presence_cb_data(cb_data);
		return G_SOURCE_REMOVE;
	}

	if (presence->cb)
		presence->cb(presence, IOTCON_ERROR_NONE, cb_data->response, presence->user_data);

	icl_destroy_presence_cb_data(cb_data);
	return G_SOURCE_REMOVE;
}

OCStackApplicationResult icl_ioty_ocprocess_presence_cb(void *ctx,
		OCDoHandle handle, OCClientResponse *resp)
{
	FN_CALL;
	int ret;
	icl_presence_cb_s *presence_cb_data;
	iotcon_presence_h presence = (iotcon_presence_h)ctx;
	iotcon_presence_response_h presence_response = NULL;

	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == presence, OC_STACK_KEEP_TRANSACTION);

	ret = icl_ioty_parse_oic_presence_payload(&resp->devAddr,
			(OCPresencePayload*)resp->payload, resp->result, &presence_response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_presence_payload() Fail(%d)", ret);
		return OC_STACK_KEEP_TRANSACTION;
	}

	ret = icl_create_presence_cb_data(presence, presence_response, &presence_cb_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_create_presence_cb_data() Fail(%d)", ret);
		icl_destroy_presence_response(presence_response);
		return OC_STACK_KEEP_TRANSACTION;
	}

	g_idle_add(_icl_ioty_ocprocess_presence_idle_cb, presence_cb_data);

	return OC_STACK_KEEP_TRANSACTION;
}

static gboolean _icl_ioty_ocprocess_observe_idle_cb(gpointer p)
{
	icl_observe_cb_s *cb_data = p;

	RETV_IF(NULL == cb_data, G_SOURCE_REMOVE);

	if (cb_data->cb)
		cb_data->cb(cb_data->resource, IOTCON_ERROR_NONE, cb_data->sequence_number,
				cb_data->response, cb_data->user_data);

	icl_destroy_observe_cb_data(cb_data);

	return G_SOURCE_REMOVE;
}


OCStackApplicationResult icl_ioty_ocprocess_observe_cb(void *ctx,
		OCDoHandle handle, OCClientResponse* resp)
{
	FN_CALL;
	int ret, response_result;
	iotcon_options_h options;
	iotcon_response_h response;
	iotcon_representation_h repr;
	icl_observe_cb_s *observe_cb_data;
	icl_observe_container_s *cb_container = ctx;
	OCStackApplicationResult cb_result;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);

	cb_result = (OC_OBSERVE_DEREGISTER == resp->sequenceNumber) ?
			OC_STACK_DELETE_TRANSACTION : OC_STACK_KEEP_TRANSACTION;

	if (NULL == resp->payload) {
		ERR("payload is empty(%d)", resp->result);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* representation */
	ret = icl_ioty_parse_oic_rep_payload((OCRepPayload*)resp->payload, true, &repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_rep_payload() Fail(%d)", ret);
		return OC_STACK_KEEP_TRANSACTION;
	}

	if (NULL == repr->uri_path && resp->resourceUri)
		repr->uri_path = ic_utils_strdup(resp->resourceUri);

	/* response result */
	response_result = ic_ioty_parse_oic_response_result(resp->result);

	/* header options */
	ret = icl_ioty_parse_oic_header_option(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions, &options);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_header_option() Fail(%d)", ret);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* response */
	response = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == response) {
		ERR("calloc() Fail(%d)", errno);
		if (options)
			iotcon_options_destroy(options);
		iotcon_representation_destroy(repr);
		return OC_STACK_KEEP_TRANSACTION;
	}
	response->header_options = options;
	response->result = (iotcon_response_result_e)response_result;
	response->repr = repr;

	ret = icl_create_observe_cb_data(cb_container, resp->sequenceNumber, response,
			&observe_cb_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_create_observe_cb_data() Fail(%d)", ret);
		iotcon_response_destroy(response);
		return OC_STACK_KEEP_TRANSACTION;
	}

	g_idle_add(_icl_ioty_ocprocess_observe_idle_cb, observe_cb_data);

	/* DO NOT FREE ctx(cb_container). It MUST be freed in the ocstack */
	/* DO NOT FREE cb_data. It MUST be freed in the idle */

	return cb_result;
}

static gboolean _icl_ioty_ocprocess_crud_idle_cb(gpointer p)
{
	icl_response_cb_s *cb_data = p;

	RETV_IF(NULL == cb_data, G_SOURCE_REMOVE);

	if (cb_data->cb)
		cb_data->cb(cb_data->resource, IOTCON_ERROR_NONE, cb_data->req_type,
				cb_data->response, cb_data->user_data);

	icl_destroy_response_cb_data(cb_data);

	return G_SOURCE_REMOVE;
}

OCStackApplicationResult icl_ioty_ocprocess_crud_cb(void *ctx,
		OCDoHandle handle, OCClientResponse *resp)
{
	FN_CALL;
	int ret;
	iotcon_response_result_e response_result;
	iotcon_options_h options;
	icl_response_container_s *cb_container = ctx;
	iotcon_response_h response;
	iotcon_representation_h repr;
	icl_response_cb_s *response_cb_data;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_DELETE_TRANSACTION);

	/* remove timeout */
	if (cb_container->timeout) {
		g_source_remove(cb_container->timeout);
		cb_container->timeout = 0;
	}

	if (NULL == resp->payload) {
		ERR("payload is empty(%d)", resp->result);
		if (cb_container->cb) {
			cb_container->cb(cb_container->resource, IOTCON_ERROR_IOTIVITY,
					cb_container->req_type, NULL, cb_container->user_data);
		}
		return OC_STACK_DELETE_TRANSACTION;
	}

	/* representation */
	ret = icl_ioty_parse_oic_rep_payload((OCRepPayload*)resp->payload, true, &repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_rep_payload() Fail(%d)", ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	if (NULL == repr->uri_path && resp->resourceUri)
		repr->uri_path = ic_utils_strdup(resp->resourceUri);

	/* response result */
	response_result = ic_ioty_parse_oic_response_result(resp->result);

	/* header options */
	ret = icl_ioty_parse_oic_header_option(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions, &options);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_header_option() Fail(%d)", ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	/* response */
	response = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == response) {
		ERR("calloc() Fail(%d)", errno);
		if (options)
			iotcon_options_destroy(options);
		iotcon_representation_destroy(repr);
		return OC_STACK_DELETE_TRANSACTION;
	}
	response->header_options = options;
	response->result = response_result;
	response->repr = repr;

	ret = icl_create_response_cb_data(cb_container, response, &response_cb_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_create_observe_cb_data() Fail(%d)", ret);
		iotcon_response_destroy(response);
		return OC_STACK_DELETE_TRANSACTION;
	}

	g_idle_add(_icl_ioty_ocprocess_crud_idle_cb, response_cb_data);

	/* DO NOT FREE ctx(cb_container). It MUST be freed in the ocstack */
	/* DO NOT FREE cb_data. It MUST be freed in the idle */

	return OC_STACK_DELETE_TRANSACTION;
}

static gboolean _icl_ioty_ocprocess_request_idle_cb(gpointer p)
{
	iotcon_resource_h resource;
	iotcon_request_h request;
	icl_request_container_s *req_container = p;

	RETV_IF(NULL == req_container, IOTCON_ERROR_INVALID_PARAMETER);

	resource = req_container->resource;
	request = req_container->request;

	resource->cb(resource, request, resource->user_data);
	icl_destroy_request_container(req_container);

	return G_SOURCE_REMOVE;
}

OCEntityHandlerResult icl_ioty_ocprocess_request_cb(OCEntityHandlerFlag flag,
		OCEntityHandlerRequest *request, void *user_data)
{
	FN_CALL;
	int ret, conn_type, observe_id = 0;
	char *host_address;
	iotcon_observe_type_e obs_type;
	iotcon_request_type_e req_type;
	iotcon_options_h options;
	iotcon_query_h query;
	iotcon_representation_h repr;
	iotcon_resource_h resource = user_data;
	char *token, *save_ptr1, *save_ptr2;
	char *query_str, *query_key, *query_value;
	iotcon_request_h req;

	RETV_IF(NULL == resource, OC_EH_ERROR);
	RETV_IF(NULL == request, OC_EH_ERROR);

	/* request type */
	if (OC_REQUEST_FLAG & flag) {
		req_type = ic_ioty_parse_oic_method(request->method);
	} else {
		ERR("Invalid flag(%d)", flag);
		return OC_EH_ERROR;
	}

	/* observe type */
	if (OC_OBSERVE_FLAG & flag) {
		observe_id = request->obsInfo.obsId;
		obs_type = ic_ioty_parse_oic_action(request->obsInfo.action);
	} else {
		obs_type = IOTCON_OBSERVE_NO_TYPE;
	}

	/* header options */
	ret = icl_ioty_parse_oic_header_option(request->rcvdVendorSpecificHeaderOptions,
			request->numRcvdVendorSpecificHeaderOptions, &options);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_oic_header_option() Fail(%d)", ret);
		return OC_EH_ERROR;
	}

	/* query */
	if (request->query && *request->query) {
		iotcon_query_create(&query);
		query_str = request->query;
		while ((token = strtok_r(query_str, "&;", &save_ptr1))) {
			while ((query_key = strtok_r(token, "=", &save_ptr2))) {
				token = NULL;
				query_value = strtok_r(token, "=", &save_ptr2);
				if (NULL == query_value)
					break;
				iotcon_query_add(query, query_key, query_value);
			}
			query_str = NULL;
		}
	} else {
		query = NULL;
	}

	/* representation */
	if (request->payload) {
		icl_ioty_parse_oic_rep_payload((OCRepPayload*)request->payload, true, &repr);
		if (repr && NULL == repr->uri_path)
			repr->uri_path = ic_utils_strdup(resource->uri_path);
	} else {
		repr = NULL;
	}


	/* for iotcon_resource_notify */
	if (IOTCON_OBSERVE_REGISTER == obs_type) {
		if (NULL == resource->observers)
			iotcon_observers_create(&resource->observers);
		iotcon_observers_add(resource->observers, observe_id);
	} else if (IOTCON_OBSERVE_DEREGISTER == obs_type) {
		iotcon_observers_remove(resource->observers, observe_id);
	}

	ic_ioty_parse_oic_dev_address(&request->devAddr, &host_address, &conn_type);

	req = calloc(1, sizeof(struct icl_resource_request));
	if (NULL == req) {
		ERR("calloc() Fail(%d)", errno);
		free(host_address);
		if (options)
			iotcon_options_destroy(options);
		if (query)
			iotcon_query_destroy(query);
		if (repr)
			iotcon_representation_destroy(repr);
		return OC_EH_ERROR;
	}

	req->connectivity_type = conn_type;
	req->host_address = host_address;
	req->type = req_type;
	req->header_options = options;
	req->query = query;
	req->observation_info.action = obs_type;
	req->observation_info.observe_id = observe_id;
	req->repr = repr;
	req->oic_request_h = IC_POINTER_TO_INT64(request->requestHandle);
	req->oic_resource_h = IC_POINTER_TO_INT64(request->resource);

	icl_request_container_s *req_container = calloc(1, sizeof(icl_request_container_s));
	req_container->request = req;
	req_container->resource = resource;

	g_idle_add(_icl_ioty_ocprocess_request_idle_cb, req_container);

	return OC_EH_OK;
}

static int _icl_ioty_ocprocess_lite_resource_get_repr(
		iotcon_lite_resource_h resource, iotcon_representation_h *representation)
{
	int ret;
	iotcon_representation_h repr;
	iotcon_resource_interfaces_h ifaces;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == representation, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_representation_create(&repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return ret;
	}

	ret = iotcon_representation_set_uri_path(repr, resource->uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_uri_path() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return ret;
	}

	ret = iotcon_representation_set_state(repr, resource->state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return ret;
	}

	ret = iotcon_resource_interfaces_create(&ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_interfaces_create() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return ret;
	}

	ret = iotcon_resource_interfaces_add(ifaces, IOTCON_INTERFACE_DEFAULT);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_interfaces_add() Fail(%d)", ret);
		iotcon_resource_interfaces_destroy(ifaces);
		iotcon_representation_destroy(repr);
		return ret;
	}

	ret = iotcon_representation_set_resource_interfaces(repr, ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_resource_interfaces() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return ret;
	}
	iotcon_resource_interfaces_destroy(ifaces);

	*representation = repr;
	return IOTCON_ERROR_NONE;
}

static gboolean _icl_ioty_ocprocess_lite_resource_response_idle_cb(gpointer p)
{
	int ret;

	ret = icl_ioty_response_send(p);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_response_send() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}
	iotcon_response_destroy(p);
	return G_SOURCE_REMOVE;
}

static gboolean _icl_ioty_ocprocess_lite_resource_notify_idle_cb(gpointer p)
{
	int ret;

	ret = icl_ioty_lite_resource_notify(p);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_lite_resource_notify() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}
	return G_SOURCE_REMOVE;
}

OCEntityHandlerResult icl_ioty_ocprocess_lite_request_cb(OCEntityHandlerFlag flag,
		OCEntityHandlerRequest *request, void *user_data)
{
	FN_CALL;
	iotcon_request_type_e req_type;
	iotcon_lite_resource_h resource = user_data;
	iotcon_representation_h repr;
	iotcon_response_h res = NULL;

	RETV_IF(NULL == resource, OC_EH_ERROR);
	RETV_IF(NULL == request, OC_EH_ERROR);

	/* request type */
	if (OC_REQUEST_FLAG & flag)
		req_type = ic_ioty_parse_oic_method(request->method);
	else {
		ERR("Invalid flag(%d)", flag);
		return OC_EH_ERROR;
	}

	/* representation */
	if (request->payload)
		icl_ioty_parse_oic_rep_payload((OCRepPayload*)request->payload, true, &repr);
	else
		repr = NULL;

	res = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == res) {
		ERR("calloc() Fail(%d)", errno);
		return OC_EH_ERROR;
	}
	res->oic_request_h = IC_POINTER_TO_INT64(request->requestHandle);
	res->oic_resource_h = IC_POINTER_TO_INT64(request->resource);
	res->iface = strdup(IOTCON_INTERFACE_DEFAULT);

	switch (req_type) {
	case IOTCON_REQUEST_GET:
		res->result = IOTCON_RESPONSE_OK;
		_icl_ioty_ocprocess_lite_resource_get_repr(resource, &(res->repr));
		g_idle_add(_icl_ioty_ocprocess_lite_resource_response_idle_cb, res);
		break;
	case IOTCON_REQUEST_POST:
		if (resource->cb) {
			if (false == resource->cb(resource, repr->state, resource->cb_data)) {
				res->result = IOTCON_RESPONSE_ERROR;
				g_idle_add(_icl_ioty_ocprocess_lite_resource_response_idle_cb, res);
				break;
			}
		}
		iotcon_state_destroy(resource->state);
		resource->state = repr->state;
		repr->state = NULL;
		_icl_ioty_ocprocess_lite_resource_get_repr(resource, &(res->repr));
		res->result = IOTCON_RESPONSE_OK;
		g_idle_add(_icl_ioty_ocprocess_lite_resource_response_idle_cb, res);
		g_idle_add(_icl_ioty_ocprocess_lite_resource_notify_idle_cb, resource);
		break;
	case IOTCON_REQUEST_PUT:
	case IOTCON_REQUEST_DELETE:
	default:
		WARN("Not supported request (only GET / POST / OBSERVE)");
		res->result = IOTCON_RESPONSE_FORBIDDEN;
		g_idle_add(_icl_ioty_ocprocess_lite_resource_response_idle_cb, res);
		break;
	}


	return OC_EH_OK;
}


