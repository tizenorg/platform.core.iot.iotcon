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

#include "iotcon-constant.h"

typedef struct ic_value_s* iotcon_value_h;
typedef struct ic_list_s* iotcon_list_h;
typedef struct ic_repr_s* iotcon_repr_h;

typedef struct ic_notify_msg* iotcon_notimsg_h;

typedef void* iotcon_presence_h;

typedef struct _device_info {
	char *name;
	char *host_name;
	char *uuid;
	char *content_type;
	char *version;
	char *manuf_name;
	char *manuf_url;
	char *model_number;
	char *date_of_manufacture;
	char *platform_ver;
	char *firmware_ver;
	char *support_url;
} iotcon_device_info_s;


typedef struct ic_options* iotcon_options_h;
iotcon_options_h iotcon_options_new();
void iotcon_options_free(iotcon_options_h options);
int iotcon_options_insert(iotcon_options_h options, unsigned short id,
		const char *data);
int iotcon_options_delete(iotcon_options_h options, unsigned short id);
const char* iotcon_options_lookup(iotcon_options_h options, unsigned short id);
typedef int (*iotcon_options_foreach_cb)(unsigned short id, const char *data,
		void *user_data);
int iotcon_options_foreach(iotcon_options_h options, iotcon_options_foreach_cb cb,
		void *user_data);


typedef struct ic_query* iotcon_query_h;
iotcon_query_h iotcon_query_new();
void iotcon_query_free(iotcon_query_h query);
int iotcon_query_insert(iotcon_query_h query, const char *key, const char *value);
int iotcon_query_delete(iotcon_query_h query, const char *key);
const char* iotcon_query_lookup(iotcon_query_h query, const char *key);
typedef int (*iotcon_query_foreach_cb)(const char *key, const char *value,
		void *user_data);
int iotcon_query_foreach(iotcon_query_h query, iotcon_query_foreach_cb cb,
		void *user_data);
iotcon_query_h iotcon_query_clone(iotcon_query_h query);


/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Appends resource type to list.
 * @since_tizen 3.0
 * @remarks  Duplicate resource types are not allowed.
 *
 * @param[in] res_types The handle to the list
 * @param[in] type The resource type
 *
 * @return the (possibly changed) start of the list, otherwise a null pointer on failure
 */
typedef struct ic_resource_types* iotcon_resource_types_h;
iotcon_resource_types_h iotcon_resource_types_new();
void iotcon_resource_types_free(iotcon_resource_types_h types);
int iotcon_resource_types_insert(iotcon_resource_types_h types, const char *type);
int iotcon_resource_types_delete(iotcon_resource_types_h types, const char *type);
typedef int (*iotcon_resource_types_foreach_cb)(const char *type, void *user_data);
int iotcon_resource_types_foreach(iotcon_resource_types_h types,
		iotcon_resource_types_foreach_cb cb, void *user_data);
iotcon_resource_types_h iotcon_resource_types_clone(iotcon_resource_types_h types);


typedef void* iotcon_observers_h;
void iotcon_observers_free(iotcon_observers_h observers);
iotcon_observers_h iotcon_observers_append(iotcon_observers_h observers, int obs_id);
iotcon_observers_h iotcon_observers_remove(iotcon_observers_h observers, int obs_id);

typedef struct ic_resource* iotcon_resource_h;
int iotcon_resource_get_number_of_children(iotcon_resource_h resource, int *number);
int iotcon_resource_get_nth_child(iotcon_resource_h parent, int index,
		iotcon_resource_h *child);
int iotcon_resource_get_uri(iotcon_resource_h resource, char **uri);
int iotcon_resource_get_host(iotcon_resource_h resource, char **host);
int iotcon_resource_get_types(iotcon_resource_h resource, iotcon_resource_types_h *types);
int iotcon_resource_get_interfaces(iotcon_resource_h resource, int *ifaces);
int iotcon_resource_is_observable(iotcon_resource_h resource, bool *observable);

typedef struct ic_remote_resource* iotcon_client_h;
int iotcon_client_get_uri(iotcon_client_h resource, char **uri);
int iotcon_client_get_host(iotcon_client_h resource, char **host);
int iotcon_client_get_types(iotcon_client_h resource, iotcon_resource_types_h *types);
int iotcon_client_get_interfaces(iotcon_client_h resource, int *ifaces);
int iotcon_client_is_observable(iotcon_client_h resource, bool *observable);
int iotcon_client_set_options(iotcon_client_h resource, iotcon_options_h header_options);

typedef struct ic_resource_request* iotcon_request_h;
int iotcon_request_get_representation(iotcon_request_h request, iotcon_repr_h *repr);
int iotcon_request_get_types(iotcon_request_h request, int *types);
int iotcon_request_get_options(iotcon_request_h request, iotcon_options_h *options);
int iotcon_request_get_query(iotcon_request_h request, iotcon_query_h *query);
int iotcon_request_get_observer_action(iotcon_request_h request,
		iotcon_observe_action_e *action);
int iotcon_request_get_observer_id(iotcon_request_h request, int *observer_id);

typedef struct ic_resource_response* iotcon_response_h;
iotcon_response_h iotcon_response_new(iotcon_request_h request_h);
void iotcon_response_free(iotcon_response_h resp);
int iotcon_response_set(iotcon_response_h resp, iotcon_response_property_e prop, ...);

#endif /* __IOT_CONNECTIVITY_MANAGER_STRUCT_H__ */
