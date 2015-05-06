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

#include "iotcon.h"

typedef struct _resource_handler_s {
	char *rt_name;
	char *if_name;
	char *uri_name;
	iotcon_rest_api_handle_cb rest_api_cb;
} resource_handler_s;

void ic_iotivity_config(const char *addr, unsigned short port);

void* ic_ioty_register_res(const char *uri, const char *rt,
		iotcon_interface_e iface, iotcon_resource_property_e rt_property);

int ic_ioty_unregister_res(const iotcon_resource_h resource_handle);

int ic_ioty_bind_iface_to_res(const iotcon_resource_h resource_handle,
		const char *interface_type);

int ic_ioty_bind_type_to_res(const iotcon_resource_h resource_handle,
		const char *resource_type);

int ic_ioty_bind_res(iotcon_resource_h parent, iotcon_resource_h child);

int ic_ioty_register_device_info(iotcon_device_info_s *device_info);

int ic_ioty_get_device_info(char *host, char *uri);

int ic_ioty_send_notify(struct ic_res_response_s *resp,
		iotcon_observers observers);

int ic_ioty_send_res_response_data(struct ic_res_response_s *resp);

iotcon_presence_h ic_ioty_subscribe_presence(const char *host_address,
		iotcon_presence_handle_cb presence_handler_cb, void *user_data);

int ic_ioty_unsubscribe_presence(iotcon_presence_h presence_handle);
int ic_ioty_start_presence(const unsigned int time_to_live);
int ic_ioty_stop_presence();

int ic_ioty_find_resource(const char *host, const char *resource_name,
		iotcon_found_resource_cb found_resource_cb, void *user_data);

int ic_ioty_get(iotcon_resource_s resource,	iotcon_query query,
		iotcon_on_get_cb on_get_cb, void *user_data);

int ic_ioty_put(iotcon_resource_s resource, iotcon_repr_h repr, iotcon_query query,
		iotcon_on_put_cb on_put_cb, void *user_data);

int ic_ioty_post(iotcon_resource_s resource, iotcon_repr_h repr, iotcon_query query,
		iotcon_on_put_cb on_post_cb, void *user_data);

int ic_ioty_delete_res(iotcon_resource_s resource,
		iotcon_on_delete_cb on_delete_cb, void *user_data);

int ic_ioty_observe(iotcon_resource_s *resource, iotcon_observe_type_e observe_type,
		iotcon_query query, iotcon_on_observe_cb on_observe_cb, void *user_data);

int ic_ioty_cancel_observe(iotcon_resource_s resource);

/**
 * @brief hash-table for resource callback handler
 */
typedef struct _iot_ctx {
	GHashTable *entity_cb_hash;
	GList *found_device_cb_lst;
} ic_ctx_s;

ic_ctx_s* ic_get_ctx();

#endif //__IOT_CONNECTIVITY_MANAGER_INTERNAL_IOTIVITY_H__
