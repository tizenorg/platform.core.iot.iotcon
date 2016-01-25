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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_H__

#include <glib.h>
#include <octypes.h>

#include "iotcon-client.h"
#include "iotcon-types.h"

typedef enum {
	ICL_FIND_RESOURCE,
	ICL_GET_DEVICE_INFO,
	ICL_GET_PLATFORM_INFO,
	ICL_ADD_PRESENCE,
} icl_operation_e;

typedef struct {
	icl_operation_e op;
	void *cb;
	void *user_data;
	bool found;
	int timeout;
	OCDoHandle handle;
} icl_cb_s;

void icl_ioty_csdk_lock();
void icl_ioty_csdk_unlock();

void icl_ioty_deinit(GThread *thread);
GThread* icl_ioty_init();

/* client APIs */
int icl_ioty_find_resource(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *resource_type,
		bool is_secure,
		iotcon_found_resource_cb cb,
		void *user_data);

int icl_ioty_get_device_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_device_info_cb cb,
		void *user_data);

int icl_ioty_get_platform_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_platform_info_cb cb,
		void *user_data);

int icl_ioty_add_presence_cb(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *resource_type,
		iotcon_presence_cb cb,
		void *user_data,
		iotcon_presence_h *presence_handle);


/* server APIs */

/*

OCResourceHandle icd_ioty_register_resource(const char *uri_path,
		const char* const* res_types, int ifaces, int properties);

int icl_ioty_unregister_resource(OCResourceHandle handle);
int icl_ioty_bind_interface(OCResourceHandle handle, iotcon_interface_e iface);
int icl_ioty_bind_type(OCResourceHandle handle, const char *resource_type);
int icl_ioty_bind_resource(OCResourceHandle parent, OCResourceHandle child);
int icl_ioty_unbind_resource(OCResourceHandle parent, OCResourceHandle child);
int icl_ioty_notify(OCResourceHandle handle, GVariant *msg, GVariant *observers, gint qos);
int icl_ioty_send_response(GVariant *resp);

void icl_ioty_complete(int type, GDBusMethodInvocation *invocation, GVariant *value);
void icl_ioty_complete_error(int type, GDBusMethodInvocation *invocation, int ret_val);

gboolean icl_ioty_get(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *query);
gboolean icl_ioty_put(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *repr, GVariant *query);
gboolean icl_ioty_post(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *repr, GVariant *query);
gboolean icl_ioty_delete(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource);
OCDoHandle icl_ioty_observer_start(GVariant *resource, int observe_type,
		GVariant *query, int64_t signal_number, const char *bus_name);

int icl_ioty_observer_stop(OCDoHandle handle, GVariant *options);


int icl_ioty_set_device_info();
int icl_ioty_set_platform_info();

OCDoHandle icl_ioty_presence_table_get_handle(const char *host_address);

OCDoHandle icl_ioty_subscribe_presence(int type, const char *host_address, int conn_type,
		const char *resource_type, void *context);

int icl_ioty_unsubscribe_presence(OCDoHandle handle, const char *host_address);

int icl_ioty_start_presence(unsigned int time_to_live);

int icl_ioty_stop_presence();

int icl_ioty_encap_get_time_interval();

void icl_ioty_encap_set_time_interval(int time_interval);

icd_encap_info_s* _icl_ioty_encap_table_get_info(const char *uri_path,
		const char *host_address);

gboolean icl_ioty_encap_get(gpointer user_data);

int icl_ioty_start_encap(int type, const char *uri_path, const char *host_address,
		int conn_type, int64_t *signal_number);

int icl_ioty_stop_encap(int type, const char  *uri_path, const char *host_address);

static inline int icl_ioty_convert_error(int ret)
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
*/

#endif /*__IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_H__*/

