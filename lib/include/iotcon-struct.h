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
#ifndef __IOT_CONNECTIVITY_MANAGER_STRUCT_H__
#define __IOT_CONNECTIVITY_MANAGER_STRUCT_H__

#include <stdbool.h>
#include <glib.h>

#include "iotcon-constant.h"

typedef struct ic_value_s* iotcon_value_h;
typedef struct ic_list_s* iotcon_list_h;
typedef struct ic_repr_s* iotcon_repr_h;

/**
 * @brief Handle to an OCResource object owned by the OCStack.
 */
typedef void* iotcon_resource_h;
typedef void* iotcon_request_h;
typedef void* iotcon_presence_h;
typedef void* iotcon_observe_h;

typedef GList* iotcon_observers;
typedef unsigned char iotcon_observation_id;
typedef GHashTable* iotcon_header_options;
typedef GHashTable* iotcon_query_parameters;

typedef GList *iotcon_resource_types;
typedef GList *iotcon_resource_interfaces;


typedef struct ic_res_response_s* iotcon_response_h;

/**
 * @brief These are used in setting header options.
 */
typedef struct {
	unsigned short option_id;
	char *option_data;
} iotcon_header_option;


/**
 * @brief observation information structure
 */
typedef struct {
	// Action associated with observation request
	iotcon_osbserve_action_e action;
	// Identifier for observation being registered/deregistered
	iotcon_observation_id obs_id;
} iotcon_observation_info_s;


typedef struct {
	char *resource_uri;
	char *resource_host;
	bool is_observable;
	bool is_collection;
	iotcon_header_options header_options;
	iotcon_resource_types resource_types;
	iotcon_resource_interfaces resource_interfaces;
	iotcon_observe_h observe_handle;
} iotcon_resource_s;


typedef struct {
	char *request_type;
	char *res_uri;
	iotcon_header_options header_opts;
	iotcon_query_parameters query_params;
	int request_handler_flag;
	iotcon_request_h request_handle;
	iotcon_resource_h resource_handle;
	iotcon_observation_info_s observation_info;
	iotcon_repr_h repr;
} iotcon_request_s;


/**
 * @brief Following structure describes the device properties.
 *        All non-Null properties will be included in a device discovery request.
 */
typedef struct
{
	char *device_name;
	char *host_name;
	char *device_uuid;
	char *content_type;
	char *version;
	char *manufacturer_name;
	char *manufacturer_url;
	char *model_number;
	char *date_of_manufacture;
	char *platform_version;
	char *firmware_version;
	char *support_url;
} iotcon_device_info_s;
#endif //__IOT_CONNECTIVITY_MANAGER_STRUCT_H__
