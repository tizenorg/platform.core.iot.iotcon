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

#include <stdint.h>
#include <iotcon-constant.h>

typedef struct ic_value_s* iotcon_value_h;
typedef struct ic_list_s* iotcon_list_h;
typedef struct ic_repr_s* iotcon_repr_h;

typedef struct _resource_s* iotcon_resource_h;
typedef struct ic_notify_msg* iotcon_notimsg_h;

typedef void* iotcon_presence_h;

typedef struct _str_list {
	char *string;
	struct _str_list *next;
} iotcon_str_list_s;


typedef struct ic_options* iotcon_options_h;
iotcon_options_h iotcon_options_new();
void iotcon_options_free(iotcon_options_h options);
int iotcon_options_insert(iotcon_options_h options, unsigned short id,
		const char *data);
int iotcon_options_delete(iotcon_options_h options, unsigned short id);
const char* iotcon_options_lookup(iotcon_options_h options, unsigned short id);
typedef void (*iotcon_options_foreach_cb)(unsigned short id, const char *data,
		void *user_data);
void iotcon_options_foreach(iotcon_options_h options,
		iotcon_options_foreach_cb cb, void *user_data);


typedef struct ic_query* iotcon_query_h;
iotcon_query_h iotcon_query_new();
void iotcon_query_free(iotcon_query_h query);
int iotcon_query_insert(iotcon_query_h query, const char *key, const char *value);
int iotcon_query_delete(iotcon_query_h query, const char *key);
const char* iotcon_query_lookup(iotcon_query_h query, const char *key);
typedef void (*iotcon_query_foreach_cb)(const char *key, const char *value,
		void *user_data);
void iotcon_query_foreach(iotcon_query_h query, iotcon_query_foreach_cb cb,
		void *user_data);
iotcon_query_h iotcon_query_clone(iotcon_query_h query);

typedef void* iotcon_observers_h;
void iotcon_observers_free(iotcon_observers_h observers);
iotcon_observers_h iotcon_observers_append(iotcon_observers_h observers, uint8_t obs_id);
iotcon_observers_h iotcon_observers_remove(iotcon_observers_h observers, uint8_t obs_id);

typedef struct ic_remote_resource* iotcon_client_h;
const char* iotcon_client_get_uri(iotcon_client_h resource);
const char* iotcon_client_get_host(iotcon_client_h resource);
iotcon_str_list_s* iotcon_client_get_types(iotcon_client_h resource);
int iotcon_client_get_interfaces(iotcon_client_h resource);
int iotcon_client_set_options(iotcon_client_h resource,
		iotcon_options_h header_options);

typedef struct ic_resource_request* iotcon_request_h;
iotcon_repr_h iotcon_request_get_representation(iotcon_request_h request);
const char* iotcon_request_get_request_type(iotcon_request_h request);
int iotcon_request_get_request_handler_flag(iotcon_request_h request);
iotcon_options_h iotcon_request_get_options(iotcon_request_h request);
iotcon_query_h iotcon_request_get_query(iotcon_request_h request);
iotcon_observe_action_e iotcon_request_get_observer_action(iotcon_request_h request);
uint8_t iotcon_request_get_observer_id(iotcon_request_h request);

typedef struct ic_resource_response* iotcon_response_h;
iotcon_response_h iotcon_response_new(iotcon_request_h request_h);
void iotcon_response_free(iotcon_response_h resp);
int iotcon_response_set(iotcon_response_h resp, iotcon_response_property_e prop, ...);

typedef struct ic_device_info* iotcon_device_info_h;
const char* iotcon_device_info_get_device_name(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_host_name(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_device_uuid(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_content_type(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_version(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_manufacturer_name(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_manufacturer_url(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_model_number(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_date_of_manufacture(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_platform_version(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_firmware_version(iotcon_device_info_h device_info);
const char* iotcon_device_info_get_support_url(iotcon_device_info_h device_info);

void iotcon_str_list_free(iotcon_str_list_s *str_list);
iotcon_str_list_s* iotcon_str_list_append(iotcon_str_list_s *str_list,
		const char *string);
iotcon_str_list_s* iotcon_str_list_remove(iotcon_str_list_s *str_list,
		const char *string);
iotcon_str_list_s* iotcon_str_list_clone(iotcon_str_list_s *str_list);
unsigned int iotcon_str_list_length(iotcon_str_list_s *str_list);
typedef void (*iotcon_string_foreach_cb)(const char *string, void *user_data);
void iotcon_str_list_foreach(iotcon_str_list_s *str_list,
		iotcon_string_foreach_cb cb, void *user_data);
const char* iotcon_str_list_nth_data(iotcon_str_list_s *str_list, unsigned int n);

#endif /* __IOT_CONNECTIVITY_MANAGER_STRUCT_H__ */
