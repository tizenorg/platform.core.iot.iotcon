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
#ifndef __IOT_CONNECTIVITY_MANAGER_H__
#define __IOT_CONNECTIVITY_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <iotcon-errors.h>
#include <iotcon-struct.h>
#include <iotcon-constant.h>

void iotcon_initialize(const char *addr, unsigned short port);
void iotcon_deinitialize();

iotcon_response_h iotcon_response_new(iotcon_request_h req_h,
		iotcon_resource_h res_h);
void iotcon_response_free(iotcon_response_h resp);

typedef void (*iotcon_rest_api_handle_cb)(const iotcon_request_s *request);

iotcon_repr_h iotcon_request_get_representation(const iotcon_request_s *request);

iotcon_resource_h iotcon_register_resource(const char *uri, const char *rt,
		iotcon_interface_e iot_if, iotcon_resource_property_e rt_type,
		iotcon_rest_api_handle_cb entity_handler_cb);
int iotcon_unregister_resource(const iotcon_resource_h resource_handle);

int iotcon_bind_interface_to_resource(iotcon_resource_h resource_handle,
		const char *interface_type);
int iotcon_bind_type_to_resource(iotcon_resource_h resource_handle,
		const char *resource_type);
int iotcon_bind_resource(iotcon_resource_h parent, iotcon_resource_h child);

typedef void (*iotcon_found_device_info_cb)(const iotcon_device_info_s *info);

int iotcon_register_device_info(iotcon_device_info_s *device_info);

int iotcon_subscribe_device_info(char *host, char *uri,
		iotcon_found_device_info_cb found_cb);
void iotcon_unsubscribe_device_info(char *host, char *uri,
		iotcon_found_device_info_cb found_cb);

int iotcon_response_set(iotcon_response_h resp, iotcon_response_property_e prop, ...);

int iotcon_send_notify_response(iotcon_response_h resp, iotcon_observers observers);
int iotcon_send_resource_response(iotcon_response_h resp);

typedef void (*iotcon_presence_handle_cb)(iotcon_error_e result, const unsigned int nonce,
		const char *host_address, void *user_data);

iotcon_presence_h iotcon_subscribe_presence(const char *host_address,
		iotcon_presence_handle_cb presence_handler_cb, void *user_data);
int iotcon_unsubscribe_presence(iotcon_presence_h presence_handle);
int iotcon_start_presence(const unsigned int time_to_live);
int iotcon_stop_presence();

typedef void (*iotcon_found_resource_cb)(iotcon_resource_s *resource, void *user_data);

int iotcon_find_resource(const char *host, const char *resource_name,
		iotcon_found_resource_cb found_resource_cb, void *user_data);

typedef void (*iotcon_on_get_cb)(const iotcon_options_h header_options,
		const iotcon_repr_h repr, const int e_code, void *user_data);
typedef void (*iotcon_on_put_cb)(const iotcon_options_h header_options,
		const iotcon_repr_h repr, const int e_code, void *user_data);
typedef void (*iotcon_on_post_cb)(const iotcon_options_h header_options,
		const iotcon_repr_h repr, const int e_code, void *user_data);
typedef void (*iotcon_on_observe_cb)(const  iotcon_options_h header_options,
		const iotcon_repr_h repr, const int e_code, const int sequence_number,
		void *user_data);
typedef void (*iotcon_on_delete_cb)(const iotcon_options_h header_options,
		const int e_code, void *user_data);

iotcon_resource_s iotcon_construct_resource_object(const char *host,
												const char *uri,
												bool is_observable,
												iotcon_resource_types resource_type,
												iotcon_resource_interfaces resource_if);
void iotcon_destruct_resource_object(iotcon_resource_s *resource);
iotcon_resource_s iotcon_copy_resource(iotcon_resource_s resource);

int iotcon_get(iotcon_resource_s resource, iotcon_query query,
		iotcon_on_get_cb on_get_cb, void *user_data);
int iotcon_put(iotcon_resource_s resource, iotcon_repr_h repr, iotcon_query query,
		iotcon_on_put_cb on_put_cb, void *user_data);
int iotcon_post(iotcon_resource_s resource, iotcon_repr_h repr, iotcon_query query,
		iotcon_on_post_cb on_post_cb, void *user_data);
int iotcon_delete_resource(iotcon_resource_s resource, iotcon_on_delete_cb on_delete_cb,
		void *user_data);

int iotcon_observe(iotcon_observe_type_e observe_type,
		iotcon_resource_s *resource,
		iotcon_query query,
		iotcon_on_observe_cb on_observe_cb,
		void *user_data);
int iotcon_cancel_observe(iotcon_resource_s resource);

iotcon_resource_types iotcon_resource_types_new();
iotcon_resource_types iotcon_resource_types_insert(iotcon_resource_types resource_types,
		const char *resource_type);
void iotcon_resource_types_free(iotcon_resource_types resource_types);

iotcon_resource_interfaces iotcon_resource_interfaces_new();
iotcon_resource_interfaces iotcon_resource_interfaces_insert(
		iotcon_resource_interfaces resource_interfaces, iotcon_interface_e interface);
void iotcon_resource_interfaces_free(iotcon_resource_interfaces resource_interfaces);

iotcon_options_h iotcon_options_new();
void iotcon_options_free(iotcon_options_h options);
int iotcon_options_insert(iotcon_options_h options, const unsigned short id,
		const char *data);
int iotcon_options_delete(iotcon_options_h options, const unsigned short id);
const char* iotcon_options_lookup(iotcon_options_h options, const unsigned short id);
typedef void (*iotcon_options_foreach_cb)(unsigned short id, char *data, void *user_data);
void iotcon_options_foreach(iotcon_options_h options, iotcon_options_foreach_cb foreach_cb,
		void *user_data);
iotcon_options_h iotcon_options_clone(iotcon_options_h options);

iotcon_query iotcon_query_new();
void iotcon_query_insert(iotcon_query query, const char *key, const char *value);
void iotcon_query_free(iotcon_query query);
char* iotcon_query_lookup(iotcon_query query, const char *key);

iotcon_observers iotcon_observation_new();
iotcon_observers iotcon_observation_insert(iotcon_observers observers,
		iotcon_observation_info_s obs);
iotcon_observers iotcon_observation_delete(iotcon_observers observers,
		iotcon_observation_info_s obs);
void iotcon_observation_free(iotcon_observers observers);

char* iotcon_resource_get_uri(iotcon_resource_s resource_s);
char* iotcon_resource_get_host(iotcon_resource_s resource_s);
iotcon_resource_types iotcon_resource_get_types(iotcon_resource_s resource_s);
iotcon_resource_interfaces iotcon_resource_get_interfaces(iotcon_resource_s resource_s);

void iotcon_resource_set_options(iotcon_resource_s *resource,
		iotcon_options_h header_options);

iotcon_repr_h iotcon_repr_new();
const char* iotcon_repr_get_uri(iotcon_repr_h repr);
int iotcon_repr_set_uri(iotcon_repr_h repr, const char *uri);

typedef void (*iotcon_resourcetype_fn)(const char *res_type, void *user_data);
void iotcon_repr_get_resource_types(iotcon_repr_h repr, iotcon_resourcetype_fn fn,
		void *user_data);
int iotcon_repr_append_resource_types(iotcon_repr_h repr, const char *type);
int iotcon_repr_get_resource_types_count(iotcon_repr_h repr);

typedef void (*iotcon_interface_fn)(const char *res_if, void *user_data);
void iotcon_repr_get_resource_interfaces(iotcon_repr_h repr, iotcon_interface_fn fn,
		void *user_data);
int iotcon_repr_append_resource_interfaces(iotcon_repr_h repr, const char *interface);
int iotcon_repr_get_resource_interfaces_count(iotcon_repr_h repr);

int iotcon_repr_get_int(iotcon_repr_h repr, const char *key);
int iotcon_repr_set_int(iotcon_repr_h repr, const char *key, int ival);
bool iotcon_repr_get_bool(iotcon_repr_h repr, const char *key);
int iotcon_repr_set_bool(iotcon_repr_h repr, const char *key, bool bval);
double iotcon_repr_get_double(iotcon_repr_h repr, const char *key);
int iotcon_repr_set_double(iotcon_repr_h repr, const char *key, double dbval);
char* iotcon_repr_get_str(iotcon_repr_h repr, const char *key);
int iotcon_repr_set_str(iotcon_repr_h repr, const char *key, const char *strval);
iotcon_list_h iotcon_repr_get_list(iotcon_repr_h repr, const char *key);
int iotcon_repr_replace_list(iotcon_repr_h repr, const char *key, iotcon_list_h list);
iotcon_repr_h iotcon_repr_get_repr(iotcon_repr_h repr, const char *key);
int iotcon_repr_replace_repr(iotcon_repr_h dest, const char *key, iotcon_repr_h src);
bool iotcon_repr_is_null(iotcon_repr_h repr, const char *key);
int iotcon_repr_set_null(iotcon_repr_h repr, const char *key);

void iotcon_repr_free(iotcon_repr_h repr);

GList* iotcon_repr_get_children(iotcon_repr_h parent);
int iotcon_repr_get_children_count(iotcon_repr_h parent);
int iotcon_repr_append_child(iotcon_repr_h parent, iotcon_repr_h child);
iotcon_repr_h iotcon_repr_get_nth_child(iotcon_repr_h parent, int index);

GList* iotcon_repr_get_key_list(iotcon_repr_h repr);
int iotcon_repr_get_keys_count(iotcon_repr_h repr);

void iotcon_repr_print(iotcon_repr_h repr);

iotcon_list_h iotcon_list_new(iotcon_repr_types_e type);
iotcon_list_h iotcon_list_append_int(iotcon_list_h list, int ival);
iotcon_list_h iotcon_list_append_bool(iotcon_list_h list, bool bval);
iotcon_list_h iotcon_list_append_double(iotcon_list_h list, double dbval);
iotcon_list_h iotcon_list_append_str(iotcon_list_h list, const char *strval);
iotcon_list_h iotcon_list_append_list(iotcon_list_h dest, iotcon_list_h src);
iotcon_list_h iotcon_list_append_repr(iotcon_list_h list, iotcon_repr_h repr);
iotcon_repr_h iotcon_list_get_nth_repr(iotcon_list_h list, int index);
int iotcon_list_get_length(iotcon_list_h list);
iotcon_list_h iotcon_list_get_nth_list(iotcon_list_h list, int index);
iotcon_value_h iotcon_list_get_nth_value(iotcon_list_h list, int index);

typedef void (*iotcon_list_fn)(iotcon_value_h value, void *user_data);
void iotcon_list_foreach(iotcon_list_h list, iotcon_list_fn fn, void *user_data);

void iotcon_repr_free_list(iotcon_list_h list);

int iotcon_value_get_type(iotcon_value_h value);
int iotcon_value_get_int(iotcon_value_h value);
bool iotcon_value_get_bool(iotcon_value_h value);
double iotcon_value_get_double(iotcon_value_h value);
char* iotcon_value_get_str(iotcon_value_h value);
iotcon_list_h iotcon_value_get_list(iotcon_value_h value);
iotcon_repr_h iotcon_value_get_repr(iotcon_value_h value);

#ifdef __cplusplus
}
#endif

#endif //__IOT_CONNECTIVITY_MANAGER_H__
