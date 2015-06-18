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
#ifndef __IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_H__
#define __IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_H__

#include <stdint.h>

#include "iotcon.h"

void icd_ioty_config(const char *addr, unsigned short port);

void* icd_ioty_register_resource(const char *uri, const char* const* res_types,
		int ifaces, uint8_t properties);

int icd_ioty_unregister_resource(iotcon_resource_h resource_handle);

int icd_ioty_bind_interface(void *resource_handle, iotcon_interface_e iface);

int icd_ioty_bind_type(void *resource_handle, const char *resource_type);

int icd_ioty_bind_resource(void *parent, void *child);

int icd_ioty_unbind_resource(void *parent, void *child);

int icd_ioty_notify_list_of_observers(int resHandle, GVariant *msg, GVariant *observers);

int icd_ioty_notify_all(int resHandle);

int icd_ioty_send_response(GVariant *resp);

int icd_ioty_find_resource(const char *host_address, const char *resource_type,
		unsigned int signal_number, const char *sender);

int icd_ioty_get(GVariant *resource, GVariant *query, unsigned int signal_number,
		const char *sender);

int icd_ioty_put(GVariant *resource, const char *repr, GVariant *query,
		unsigned int signal_number, const char *sender);

int icd_ioty_post(GVariant *resource, const char *repr, GVariant *query,
		unsigned int signal_number, const char *sender);

int icd_ioty_delete(GVariant *resource, unsigned int signal_number, const char *sender);

int icd_ioty_observer_start(GVariant *resource, int observe_type, GVariant *query,
		unsigned int signal_number, const char *sender, int *observe_h);

int icd_ioty_observer_stop(void *observe_h);

int icd_ioty_register_device_info(GVariant *value);

int icd_ioty_get_device_info(const char *host_address, unsigned int signal_number,
		const char *sender);

iotcon_presence_h icd_ioty_subscribe_presence(const char *host_address,
		const char *resource_type, unsigned int signal_number, const char *sender);

int icd_ioty_unsubscribe_presence(iotcon_presence_h presence_handle);

int icd_ioty_start_presence(unsigned int time_to_live);

int icd_ioty_stop_presence();

#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_H__*/
