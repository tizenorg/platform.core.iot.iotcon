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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_STRUCT_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_STRUCT_H__

#include <stdint.h>
#include <stdbool.h>
#include <glib.h>

#include "iotcon.h"
#include "iotcon-constant.h"
#include "iotcon-struct.h"

struct ic_value_s {
	int type;
};

struct ic_list_s {
	int type;
	GList *list;
};

struct ic_repr_s {
	char *uri;
	GHashTable *hash_table;
	GList *children;
	GList *res_types;
	GList *interfaces;
};

typedef struct {
	int type;
	union {
		int i;
		bool b;
		double d;
		char *s;
	} val;
} ic_basic_s;

typedef struct {
	int type;
	struct ic_list_s *list;
} ic_val_list_s;

typedef struct {
	int type;
	struct ic_repr_s *repr;
} ic_val_repr_s;

struct ic_options {
	bool has_parent;
	GHashTable *options;
};

struct ic_observe_info {
	iotcon_observe_action_e action;
	uint8_t observer_id;
};

typedef void* oc_request_h;
typedef void* oc_resource_h;

typedef struct ic_resource {
	void *handle;
	iotcon_request_handler_cb request_handler_cb;
	void *user_data;
} resource_handler_s;

struct ic_remote_resource {
	char *uri;
	char *host;
	bool is_observable;
	bool is_collection;
	iotcon_options_h header_options;
	iotcon_str_list_s *types;
	int ifaces;
	iotcon_observe_h observe_handle;
};

struct ic_resource_request {
	char *request_type;
	char *uri;
	iotcon_options_h header_options;
	iotcon_query_h query;
	int request_handler_flag;
	struct ic_observe_info observation_info;
	iotcon_repr_h repr;
	oc_request_h request_handle;
	oc_resource_h resource_handle;
};

struct ic_resource_response {
	char *new_uri;
	int error_code;
	iotcon_options_h header_options;
	iotcon_interface_e iface;
	iotcon_response_result_e result;
	iotcon_repr_h repr;
	oc_request_h request_handle;
	oc_resource_h resource_handle;
};

struct ic_notify_msg {
	int error_code;
	iotcon_interface_e iface;
	iotcon_repr_h repr;
};

struct ic_device_info {
	char *device_name;
	char *host_name;
	char *device_uuid;
	char *content_type;
	char *version;
	char *manufacturer_name;
	char *manufacturer_url;
	char *model_number;
	char *date_of_manufacture;
	char *platform_ver;
	char *firmware_ver;
	char *support_url;
};

#endif //__IOT_CONNECTIVITY_MANAGER_INTERNAL_STRUCT_H__
