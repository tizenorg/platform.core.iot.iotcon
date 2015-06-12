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
#ifndef __IOT_CONNECTIVITY_MANAGER_DBUS_H__
#define __IOT_CONNECTIVITY_MANAGER_DBUS_H__

int icl_dbus_config(const char *address, unsigned short port);
void* icl_dbus_register_resource(const char *uri, iotcon_resource_types_h types,
		int ifaces, uint8_t properties, iotcon_request_handler_cb cb, void *user_data);
int icl_dbus_unregister_resource(void *resource);

int icl_dbus_bind_interface(void *resource, int iface);
int icl_dbus_bind_type(void *resource, const char *type);
int icl_dbus_bind_resource(void *parent, void *child);
int icl_dbus_unbind_resource(void *parent, void *child);

int icl_dbus_notify_list_of_observers(void *resource, struct ic_notify_msg *msg,
		iotcon_observers_h observers);
int icl_dbus_notify_all(void *resource);
int icl_dbus_send_response(struct ic_resource_response *response);

int icl_dbus_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb found_resource_cb, void *user_data);

int icl_dbus_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data);
int icl_dbus_put(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data);
int icl_dbus_post(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data);
int icl_dbus_delete(iotcon_client_h resource, iotcon_on_delete_cb cb,
		void *user_data);
int icl_dbus_observer_start(iotcon_client_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_on_observe_cb cb,
		void *user_data);
int icl_dbus_observer_stop(iotcon_client_h resource);

int icl_dbus_register_device_info(iotcon_device_info_s info);
int icl_dbus_get_device_info(const char *host_address, iotcon_device_info_cb cb,
		void *user_data);

int icl_dbus_start_presence(unsigned int time_to_live);
int icl_dbus_stop_presence();
void* icl_dbus_subscribe_presence(const char *host_address, const char *type,
		iotcon_presence_cb cb, void *user_data);
int icl_dbus_unsubscribe_presence(void *presence_h);

unsigned int icl_dbus_start();
void icl_dbus_stop();

#endif /* __IOT_CONNECTIVITY_MANAGER_DBUS_H__ */
