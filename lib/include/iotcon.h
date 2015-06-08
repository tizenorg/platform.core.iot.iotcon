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

#include <stdint.h>
#include <iotcon-errors.h>
#include <iotcon-struct.h>
#include <iotcon-constant.h>
#include <iotcon-representation.h>

void iotcon_initialize(const char *addr, unsigned short port);
void iotcon_deinitialize();

typedef void (*iotcon_request_handler_cb)(iotcon_request_h request, void *user_data);
iotcon_resource_h iotcon_register_resource(const char *uri,
		iotcon_resource_types_h res_types,
		int ifaces,
		uint8_t properties,
		iotcon_request_handler_cb cb,
		void *user_data);
void iotcon_unregister_resource(iotcon_resource_h resource_handle);

int iotcon_bind_interface(iotcon_resource_h resource,
		iotcon_interface_e iface);
int iotcon_bind_type(iotcon_resource_h resource_handle,
		const char *resource_type);
int iotcon_bind_request_handler(iotcon_resource_h resource, iotcon_request_handler_cb cb);
int iotcon_bind_resource(iotcon_resource_h parent, iotcon_resource_h child);
int iotcon_unbind_resource(iotcon_resource_h parent, iotcon_resource_h child);

int iotcon_register_device_info(iotcon_device_info_s device_info);
typedef void (*iotcon_device_info_cb)(iotcon_device_info_s info, void *user_data);
int iotcon_get_device_info(const char *host_address, iotcon_device_info_cb cb,
		void *user_data);

int iotcon_start_presence(unsigned int time_to_live);
int iotcon_stop_presence();
typedef void (*iotcon_presence_cb)(int result, unsigned int nonce,
		const char *host_address, void *user_data);
iotcon_presence_h iotcon_subscribe_presence(const char *host_address,
		const char *resource_type,
		iotcon_presence_cb cb,
		void *user_data);
int iotcon_unsubscribe_presence(iotcon_presence_h presence_handle);

typedef void (*iotcon_found_resource_cb)(iotcon_client_h resource, void *user_data);
int iotcon_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb cb, void *user_data);
iotcon_client_h iotcon_client_new(const char *host, const char *uri, bool is_observable,
		iotcon_resource_types_h resource_types, int resource_interfaces);
void iotcon_client_free(iotcon_client_h resource);
iotcon_client_h iotcon_client_clone(iotcon_client_h resource);

typedef void (*iotcon_on_observe_cb)(iotcon_options_h header_options, iotcon_repr_h repr,
		int response_result, int sequence_number, void *user_data);
int iotcon_observer_start(iotcon_client_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_on_observe_cb cb,
		void *user_data);
int iotcon_observer_stop(iotcon_client_h resource);

int iotcon_response_send(iotcon_response_h resp);

iotcon_notimsg_h iotcon_notimsg_new(iotcon_repr_h repr, iotcon_interface_e iface);
void iotcon_notimsg_free(iotcon_notimsg_h msg);
int iotcon_notify(iotcon_resource_h resource, iotcon_notimsg_h msg,
		iotcon_observers_h observers);

typedef void (*iotcon_on_cru_cb)(iotcon_options_h header_options, iotcon_repr_h repr,
		int response_result, void *user_data);
int iotcon_get(iotcon_client_h resource, iotcon_query_h query, iotcon_on_cru_cb cb,
		void *user_data);

int iotcon_put(iotcon_client_h resource, iotcon_repr_h repr, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data);

int iotcon_post(iotcon_client_h resource, iotcon_repr_h repr, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data);

typedef void (*iotcon_on_delete_cb)(iotcon_options_h header_options, int response_result,
		void *user_data);
int iotcon_delete(iotcon_client_h resource, iotcon_on_delete_cb cb,
		void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_MANAGER_H__ */
