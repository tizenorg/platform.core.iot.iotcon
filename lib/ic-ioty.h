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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_IOTIVITY_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_IOTIVITY_H__

#include <stdint.h>

#include "iotcon.h"
#include "iotcon-constant.h"
#include "ic-struct.h"

void ic_ioty_config(const char *addr, unsigned short port);

void* ic_ioty_register_res(const char *uri, iotcon_str_list_s *res_types,
		int ifaces, uint8_t properties);

int ic_ioty_unregister_res(iotcon_resource_h resource_handle);

int ic_ioty_bind_iface_to_res(void *resource_handle, iotcon_interface_e iface);

int ic_ioty_bind_type_to_res(void *resource_handle, const char *resource_type);

int ic_ioty_bind_res(void *parent, void *child);

int ic_ioty_register_device_info(iotcon_device_info_h device_info);

int ic_ioty_get_device_info(const char *host_address, iotcon_device_info_cb found_cb,
		void *user_data);

int ic_ioty_send_notify(void *resource, struct ic_notify_msg *msg,
		iotcon_observers_h observers);

int ic_ioty_send_res_response_data(struct ic_resource_response *resp);

const iotcon_presence_h ic_ioty_subscribe_presence(const char *host_address,
		const char *resource_type,
		iotcon_presence_cb presence_handler_cb,
		void *user_data);
int ic_ioty_unsubscribe_presence(iotcon_presence_h presence_handle);
int ic_ioty_start_presence(unsigned int time_to_live);
int ic_ioty_stop_presence();

int ic_ioty_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb found_resource_cb, void *user_data);

int ic_ioty_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_get_cb on_get_cb, void *user_data);

int ic_ioty_put(iotcon_client_h resource, iotcon_repr_h repr, iotcon_query_h query,
		iotcon_on_put_cb on_put_cb, void *user_data);

int ic_ioty_post(iotcon_client_h resource, iotcon_repr_h repr, iotcon_query_h query,
		iotcon_on_put_cb on_post_cb, void *user_data);

int ic_ioty_delete_res(iotcon_client_h resource,
		iotcon_on_delete_cb on_delete_cb, void *user_data);

int ic_ioty_observe(iotcon_client_h resource, iotcon_observe_type_e observe_type,
		iotcon_query_h query, iotcon_on_observe_cb on_observe_cb, void *user_data);

int ic_ioty_cancel_observe(iotcon_client_h resource);


#endif //__IOT_CONNECTIVITY_MANAGER_INTERNAL_IOTIVITY_H__
