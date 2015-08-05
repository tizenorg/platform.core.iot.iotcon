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

#include <stdio.h>
#include <stdint.h>
#include <glib.h>

#include "iotcon.h"

#define ICD_IOTY_COAP "coap://"
#define ICD_IOTY_COAPS "coaps://"

typedef struct {
	unsigned int signum;
	char *bus_name;
} icd_sig_ctx_s;

enum {
	ICD_CRUD_GET,
	ICD_CRUD_PUT,
	ICD_CRUD_POST,
	ICD_CRUD_DELETE
};

void icd_ioty_csdk_lock();

void icd_ioty_csdk_unlock();

GThread* icd_ioty_init(const char *addr, unsigned short port);

void icd_ioty_deinit();

void* icd_ioty_register_resource(const char *uri_path, const char* const* res_types,
		int ifaces, uint8_t properties);

int icd_ioty_unregister_resource(void *resource_handle);

int icd_ioty_bind_interface(void *resource_handle, iotcon_interface_e iface);

int icd_ioty_bind_type(void *resource_handle, const char *resource_type);

int icd_ioty_bind_resource(void *parent, void *child);

int icd_ioty_unbind_resource(void *parent, void *child);

int icd_ioty_notify_list_of_observers(void *handle, GVariant *msg, GVariant *observers);

int icd_ioty_notify_all(void *handle);

int icd_ioty_send_response(GVariant *resp);

int icd_ioty_find_resource(const char *host_address, const char *resource_type,
		unsigned int signal_number, const char *bus_name);

void icd_ioty_complete(int type, GDBusMethodInvocation *invocation, GVariant *value);
void icd_ioty_complete_error(int type, GDBusMethodInvocation *invocation, int ret_val);

gboolean icd_ioty_get(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *query);

gboolean icd_ioty_put(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, const char *repr, GVariant *query);

gboolean icd_ioty_post(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, const char *repr, GVariant *query);

gboolean icd_ioty_delete(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource);

int icd_ioty_observer_start(GVariant *resource, int observe_type, GVariant *query,
		unsigned int signal_number, const char *bus_name, int *observe_h);

int icd_ioty_observer_stop(void *observe_h);

#ifdef DEVICE_INFO_IMPL /* not implemented in iotivity 0.9.1 */
int icd_ioty_register_device_info(GVariant *value);

int icd_ioty_get_device_info(const char *host_address, unsigned int signal_number,
		const char *bus_name);
#endif

int icd_ioty_register_platform_info(GVariant *value);

int icd_ioty_get_platform_info(const char *host_address, unsigned int signal_number,
		const char *bus_name);

void* icd_ioty_subscribe_presence(const char *host_address, const char *resource_type,
		unsigned int signal_number, const char *bus_name);

int icd_ioty_unsubscribe_presence(void *presence_handle);

int icd_ioty_start_presence(unsigned int time_to_live);

int icd_ioty_stop_presence();

#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_H__*/
