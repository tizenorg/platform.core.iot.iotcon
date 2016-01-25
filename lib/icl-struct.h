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

#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_STRUCT_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_STRUCT_H__

#include "iotcon-types.h"
#include "icl-ioty.h"

typedef struct {
	iotcon_found_resource_cb cb;
	void *user_data;
	iotcon_remote_resource_h *resource_list;
	int resource_count;
} icl_find_cb_s;

typedef struct {
	iotcon_platform_info_cb cb;
	void *user_data;
	iotcon_platform_info_h platform_info;
} icl_platform_cb_s;

typedef struct {
	iotcon_device_info_cb cb;
	void *user_data;
	iotcon_device_info_h device_info;
} icl_device_cb_s;

typedef struct {
	iotcon_presence_h presence;
	iotcon_presence_response_h response;
} icl_presence_cb_s;

void icl_free_find_cb_data(icl_find_cb_s *cb_data);
int icl_create_find_cb_data(icl_cb_s *cb_data,
		iotcon_remote_resource_h *resource_list,
		int resource_count,
		icl_find_cb_s **find_callback_data);

void icl_free_device_cb_data(icl_device_cb_s *cb_data);
int icl_create_device_cb_data(icl_cb_s *cb_data,
		iotcon_device_info_h device_info, icl_device_cb_s **dev_cb_data);

void icl_free_platform_cb_data(icl_platform_cb_s *cb_data);
int icl_create_platform_cb_data(icl_cb_s *cb_data,
		iotcon_platform_info_h platform_info, icl_platform_cb_s **platform_cb_data);

void icl_free_presence_cb_data(icl_presence_cb_s *cb_data);
int icl_create_presence_cb_data(iotcon_presence_h presence,
		iotcon_presence_response_h response, icl_presence_cb_s **presence_cb_data);

void icl_free_presence(iotcon_presence_h presence);
void icl_free_presence_response(iotcon_presence_response_h presence_response);

#endif /*__IOT_CONNECTIVITY_MANAGER_LIBRARY_STRUCT_H__*/
