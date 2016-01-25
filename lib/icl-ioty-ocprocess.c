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
#include "ic-ioty-utils.h"
#include "icl.h"
#include "icl-remote-resource.h"
#include "icl-device.h"
#include "icl-presence.h"
#include "icl-response.h"
#include "icl-options.h"
#include "icl-representation.h"
#include "icl-struct.h"
#include "icl-ioty.h"
#include "icl-ioty-struct.h"
#include "icl-ioty-ocprocess.h"

static int icl_ioty_alive;

void icl_ioty_ocprocess_stop()
{
	icl_ioty_alive = 0;
}

gpointer icl_ioty_ocprocess_thread(gpointer data)
{
	FN_CALL;
	OCStackResult result;
	const struct timespec delay = {0, 10 * 1000 * 1000}; /* 10 ms */

	icl_ioty_alive = 1;
	while (icl_ioty_alive) {
		icl_ioty_csdk_lock();
		result = OCProcess();
		icl_ioty_csdk_lock();
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


static gboolean _icl_ioty_ocprocess_find_idle_cb(gpointer p)
{
	int i;
	icl_find_cb_s *cb_data = p;

	RETV_IF(NULL == cb_data, G_SOURCE_REMOVE);

	if (cb_data->cb) {
		for (i = 0; i < cb_data->resource_count; i++) {
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

	ret = icl_ioty_parse_find_payload(&(resp->devAddr), (OCDiscoveryPayload*)resp->payload,
			&resource_list, &resource_count);
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

	ret = icl_ioty_parse_device_info_payload((OCDevicePayload *)resp->payload,
			&info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_device_info_payload() Fail(%d)", ret);
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

	if (PAYLOAD_TYPE_DEVICE != resp->payload->type) {
		ERR("Invalid payload type (%d)", resp->payload->type);
		return OC_STACK_KEEP_TRANSACTION;
	}

	ret = icl_ioty_parse_platform_info_payload((OCPlatformPayload *)resp->payload,
			&info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_platform_info_payload() Fail(%d)", ret);
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

OCStackApplicationResult icl_ioty_ocprocess_presence_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	FN_CALL;
	int ret;
	icl_presence_cb_s *presence_cb_data;
	iotcon_presence_h presence = (iotcon_presence_h)ctx;
	iotcon_presence_response_h presence_response = NULL;

	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == presence, OC_STACK_KEEP_TRANSACTION);

	ret = icl_ioty_parse_presence(&resp->devAddr, (OCPresencePayload*)resp->payload,
			resp->result, &presence_response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_presence() Fail(%d)", ret);
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


static iotcon_options_h _icl_ioty_ocprocess_parse_header_options(
		OCHeaderOption *oic_option, int option_size)
{
	int i, ret;
	iotcon_options_h options;

	RETV_IF(option_size <= 0, NULL);

	ret = iotcon_options_create(&options);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_options_create() Fail(%d)", ret);
		return NULL;
	}

	for (i = 0; i < option_size; i++)
		iotcon_options_add(options, oic_option[i].optionID, (char *)oic_option[i].optionData);

	return options;
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
		ERR("payload is empty");
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* representation */
	ret = icl_ioty_parse_representation_payload((OCRepPayload*)resp->payload, true, &repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_parse_representation_payload() Fail(%d)", ret);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* response result */
	response_result = ic_ioty_utils_parse_oic_result(resp->result);

	/* header options */
	options = _icl_ioty_ocprocess_parse_header_options(resp->rcvdVendorSpecificHeaderOptions,
			resp->numRcvdVendorSpecificHeaderOptions);

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

	ret = icl_create_observe_cb_data(cb_container, resp->sequenceNumber, response, &observe_cb_data);
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

