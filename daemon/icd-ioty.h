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
#include <gio/gio.h>

#include <octypes.h>

#include "iotcon.h"
#include "ic-dbus.h"

#define ICD_IOTY_COAP "coap://"
#define ICD_IOTY_COAPS "coaps://"

#define ICD_MULTICAST_ADDRESS "224.0.1.187:5683"

typedef struct {
	int64_t signal_number;
	char *bus_name;
} icd_sig_ctx_s;

typedef struct {
	OCDoHandle handle;
	int client_count;
} icd_presence_handle_info_s;

typedef struct {
	GMutex icd_worker_mutex;
	OCDevAddr dev_addr;
	char *uri_path;
	bool presence_flag;
	bool observe_flag;
	bool is_valid;
} icd_encap_worker_ctx_s;

typedef struct {
	char *uri_path;
	OCDevAddr dev_addr;
	OCConnectivityType oic_conn_type;
	int64_t signal_number;
	int get_timer_id;
	int monitoring_count;
	int caching_count;
	OCDoHandle presence_handle;
	OCDoHandle observe_handle;
	iotcon_remote_resource_state_e resource_state;
	OCRepPayload *oic_payload;
	icd_encap_worker_ctx_s *worker_ctx;
} icd_encap_info_s;

enum {
	ICD_CRUD_GET,
	ICD_CRUD_PUT,
	ICD_CRUD_POST,
	ICD_CRUD_DELETE,
	ICD_DEVICE_INFO,
	ICD_PLATFORM_INFO,
	ICD_ENCAP_MONITORING,
	ICD_ENCAP_CACHING,
	ICD_PRESENCE,
};

void icd_ioty_csdk_lock();

void icd_ioty_csdk_unlock();

GThread* icd_ioty_init(const char *addr, unsigned short port);

void icd_ioty_deinit(GThread *thread);

OCResourceHandle icd_ioty_register_resource(const char *uri_path,
		const char* const* res_types, int ifaces, int properties);

int icd_ioty_unregister_resource(OCResourceHandle handle);

int icd_ioty_bind_interface(OCResourceHandle handle, iotcon_interface_e iface);

int icd_ioty_bind_type(OCResourceHandle handle, const char *resource_type);

int icd_ioty_bind_resource(OCResourceHandle parent, OCResourceHandle child);

int icd_ioty_unbind_resource(OCResourceHandle parent, OCResourceHandle child);

int icd_ioty_notify(OCResourceHandle handle, GVariant *msg, GVariant *observers);

int icd_ioty_send_response(GVariant *resp);

int icd_ioty_find_resource(const char *host_address,
		int conn_type,
		const char *resource_type,
		bool is_secure,
		int64_t signal_number,
		const char *bus_name);

void icd_ioty_complete(int type, GDBusMethodInvocation *invocation, GVariant *value);
void icd_ioty_complete_error(int type, GDBusMethodInvocation *invocation, int ret_val);

gboolean icd_ioty_get(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *query);

gboolean icd_ioty_put(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *repr, GVariant *query);

gboolean icd_ioty_post(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *repr, GVariant *query);

gboolean icd_ioty_delete(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource);

OCDoHandle icd_ioty_observer_start(GVariant *resource, int observe_type,
		GVariant *query, int64_t signal_number, const char *bus_name);

int icd_ioty_observer_stop(OCDoHandle handle, GVariant *options);

int icd_ioty_get_info(int type, const char *host_address, int conn_type,
		int64_t signal_number, const char *bus_name);

int icd_ioty_set_device_info();
int icd_ioty_set_platform_info();

OCDoHandle icd_ioty_presence_table_get_handle(const char *host_address);

OCDoHandle icd_ioty_subscribe_presence(int type, const char *host_address, int conn_type,
		const char *resource_type, void *context);

int icd_ioty_unsubscribe_presence(OCDoHandle handle, const char *host_address);

int icd_ioty_start_presence(unsigned int time_to_live);

int icd_ioty_stop_presence();

int icd_ioty_encap_get_time_interval();

void icd_ioty_encap_set_time_interval(int time_interval);

icd_encap_info_s* _icd_ioty_encap_table_get_info(const char *uri_path,
		const char *host_address);

gboolean icd_ioty_encap_get(gpointer user_data);

int icd_ioty_start_encap(int type, const char *uri_path, const char *host_address,
		int conn_type, int64_t *signal_number);

int icd_ioty_stop_encap(int type, const char  *uri_path, const char *host_address);

static inline int icd_ioty_convert_error(int ret)
{
	switch (ret) {
	case OC_STACK_NO_MEMORY:
		return IOTCON_ERROR_OUT_OF_MEMORY;
	case OC_STACK_NO_RESOURCE:
		return IOTCON_ERROR_NO_DATA;
	default:
		break;
	}
	return IOTCON_ERROR_IOTIVITY;
}

#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_H__*/
