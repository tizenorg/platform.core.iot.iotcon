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

#include "iotcon.h"
#include "icl.h"
#include "icl-device.h"
#include "icl-presence.h"
#include "icl-remote-resource.h"
#include "icl-struct.h"

void icl_destroy_find_cb_data(icl_find_cb_s *cb_data)
{
	int i;
	RET_IF(NULL == cb_data);

	for (i = 0; i < cb_data->resource_count; i++)
		iotcon_remote_resource_destroy(cb_data->resource_list[i]);

	free(cb_data);
}

int icl_create_find_cb_data(icl_cb_s *cb_data, iotcon_remote_resource_h *resource_list,
		int resource_count, icl_find_cb_s **find_cb_data)
{
	icl_find_cb_s *cd;

	RETV_IF(NULL == cb_data, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == find_cb_data, IOTCON_ERROR_INVALID_PARAMETER);

	cd = calloc(1, sizeof(icl_find_cb_s));
	if (NULL == cd) {
		ERR("calloc(icl_find_cb_s) Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cd->cb = (iotcon_found_resource_cb)cb_data->cb;
	cd->user_data = cb_data->user_data;
	cd->resource_list = resource_list;
	cd->resource_count = resource_count;

	*find_cb_data = cd;
	return IOTCON_ERROR_NONE;
}


void icl_destroy_device_cb_data(icl_device_cb_s *cb_data)
{
	RET_IF(NULL == cb_data);

	if (cb_data->device_info) {
		free(cb_data->device_info->device_name);
		free(cb_data->device_info->spec_ver);
		free(cb_data->device_info->device_id);
		free(cb_data->device_info->data_model_ver);
		free(cb_data->device_info);
	}

	free(cb_data);
}

int icl_create_device_cb_data(icl_cb_s *cb_data,
		iotcon_device_info_h device_info, icl_device_cb_s **dev_cb_data)
{
	icl_device_cb_s *cd = NULL;

	RETV_IF(NULL == cb_data, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device_info, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dev_cb_data, IOTCON_ERROR_INVALID_PARAMETER);

	cd = calloc(1, sizeof(icl_device_cb_s));
	if (NULL == cd) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cd->cb = (iotcon_device_info_cb)cb_data->cb;
	cd->user_data = cb_data->user_data;
	cd->device_info = device_info;

	return IOTCON_ERROR_NONE;
}

void icl_destroy_platform_cb_data(icl_platform_cb_s *cb_data)
{
	RET_IF(NULL == cb_data);

	if (cb_data->platform_info) {
		free(cb_data->platform_info->platform_id);
		free(cb_data->platform_info->manuf_name);
		free(cb_data->platform_info->manuf_url);
		free(cb_data->platform_info->model_number);
		free(cb_data->platform_info->date_of_manuf);
		free(cb_data->platform_info->platform_ver);
		free(cb_data->platform_info->os_ver);
		free(cb_data->platform_info->hardware_ver);
		free(cb_data->platform_info->firmware_ver);
		free(cb_data->platform_info->support_url);
		free(cb_data->platform_info->system_time);
		free(cb_data->platform_info);
	}

	free(cb_data);
}

int icl_create_platform_cb_data(icl_cb_s *cb_data,
		iotcon_platform_info_h platform_info, icl_platform_cb_s **platform_cb_data)
{
	icl_platform_cb_s *cd = NULL;

	RETV_IF(NULL == cb_data, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == platform_info, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == platform_cb_data, IOTCON_ERROR_INVALID_PARAMETER);

	cd = calloc(1, sizeof(icl_platform_cb_s));
	if (NULL == cd) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cd->cb = (iotcon_platform_info_cb)cb_data->cb;
	cd->user_data = cb_data->user_data;
	cd->platform_info = platform_info;

	return IOTCON_ERROR_NONE;
}

void icl_destroy_presence_cb_data(icl_presence_cb_s *cb_data)
{
	RET_IF(NULL == cb_data);
	icl_destroy_presence_response(cb_data->response);
	free(cb_data);
}

int icl_create_presence_cb_data(iotcon_presence_h presence,
		iotcon_presence_response_h response, icl_presence_cb_s **presence_cb_data)
{
	icl_presence_cb_s *cd = NULL;

	RETV_IF(NULL == presence, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == presence_cb_data, IOTCON_ERROR_INVALID_PARAMETER);

	cd = calloc(1, sizeof(icl_presence_cb_s));
	if (NULL == presence_cb_data) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cd->presence = presence;
	cd->response = response;

	*presence_cb_data = cd;

	return IOTCON_ERROR_NONE;
}

void icl_destroy_presence_response(iotcon_presence_response_h presence_response)
{
	RET_IF(NULL == presence_response);
	free(presence_response->host_address);
	free(presence_response->resource_type);
	free(presence_response);
}

void icl_destroy_presence(iotcon_presence_h presence)
{
	RET_IF(NULL == presence);
	free(presence->host_address);
	free(presence->resource_type);
	free(presence);
}

void icl_destroy_observe_cb_data(icl_observe_cb_s *cb_data)
{
	RET_IF(NULL == cb_data);
	if (cb_data->response)
		iotcon_response_destroy(cb_data->response);
	if (cb_data->resource)
		icl_remote_resource_unref(cb_data->resource);
	free(cb_data);
}

int icl_create_observe_cb_data(icl_observe_container_s *cb_container,
		int sequence_number, iotcon_response_h response, icl_observe_cb_s **observe_cb_data)
{
	icl_observe_cb_s *cb_data;

	RETV_IF(NULL == cb_container, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == observe_cb_data, IOTCON_ERROR_INVALID_PARAMETER);

	cb_data = calloc(1, sizeof(icl_observe_cb_s));
	if (NULL == cb_data) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_data->cb = cb_container->cb;
	cb_data->user_data = cb_container->user_data;
	cb_data->resource = cb_container->resource;
	icl_remote_resource_ref(cb_data->resource);
	cb_data->response = response;
	cb_data->sequence_number = sequence_number;

	*observe_cb_data = cb_data;

	return IOTCON_ERROR_NONE;
}

