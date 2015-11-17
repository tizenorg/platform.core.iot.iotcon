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
#include <stdint.h>
#include <stdlib.h>
#include <gio/gio.h>

#include <octypes.h>

#include "iotcon.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-dbus.h"
#include "icd.h"
#include "icd-ioty.h"
#include "icd-dbus.h"

static icDbus *icd_dbus_object;

/* global list to care resource handle for each client */
static GList *icd_dbus_client_list;
static GMutex icd_dbus_client_list_mutex;

typedef struct _icd_dbus_client_s {
	gchar *bus_name;
	GList *hdlist;
} icd_dbus_client_s;

typedef struct _icd_resource_handle {
	OCResourceHandle handle;
	int64_t signal_number;
} icd_resource_handle_s;


icDbus* icd_dbus_get_object()
{
	return icd_dbus_object;
}


int64_t icd_dbus_generate_signal_number()
{
	static int64_t i = 0;

	return i++;
}


static void _icd_dbus_resource_handle_free(OCResourceHandle handle)
{
	icd_dbus_client_s *client;
	GList *cur_client, *cur_hd;
	icd_resource_handle_s *rsrc_handle;

	g_mutex_lock(&icd_dbus_client_list_mutex);
	cur_client = icd_dbus_client_list;
	while (cur_client) {
		client = cur_client->data;
		if (NULL == client) {
			ERR("client is NULL");
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return;
		}

		cur_hd = client->hdlist;
		while (cur_hd) {
			rsrc_handle = cur_hd->data;

			if (rsrc_handle->handle == handle) {
				DBG("resource handle(%u, %u) removed from handle list", handle,
						rsrc_handle->signal_number);
				client->hdlist = g_list_delete_link(client->hdlist, cur_hd);
				free(rsrc_handle);
				g_mutex_unlock(&icd_dbus_client_list_mutex);
				return;
			}
			cur_hd = cur_hd->next;
		}

		cur_client = cur_client->next;
	}

	g_mutex_unlock(&icd_dbus_client_list_mutex);
	return;
}

int icd_dbus_client_list_get_info(OCResourceHandle handle,
		int64_t *signal_number, gchar **bus_name)
{
	FN_CALL;
	icd_dbus_client_s *client;
	GList *cur_client, *cur_hd;
	icd_resource_handle_s *rsrc_handle;

	RETV_IF(NULL == signal_number, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == bus_name, IOTCON_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&icd_dbus_client_list_mutex);
	cur_client = icd_dbus_client_list;
	while (cur_client) {
		client = cur_client->data;
		if (NULL == client) {
			ERR("client is NULL");
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return IOTCON_ERROR_NO_DATA;
		}

		cur_hd = client->hdlist;
		while (cur_hd) {
			rsrc_handle = cur_hd->data;

			if (rsrc_handle->handle == handle) {
				DBG("signal_number(%u) for resource handle(%u) found",
						rsrc_handle->signal_number, handle);
				*signal_number = rsrc_handle->signal_number;
				*bus_name = ic_utils_strdup(client->bus_name);
				g_mutex_unlock(&icd_dbus_client_list_mutex);
				return IOTCON_ERROR_NONE;
			}
			cur_hd = cur_hd->next;
		}

		cur_client = cur_client->next;
	}

	g_mutex_unlock(&icd_dbus_client_list_mutex);
	return IOTCON_ERROR_NO_DATA;
}

int icd_dbus_emit_signal(const char *dest, const char *signal_name, GVariant *value)
{
	gboolean ret;
	GError *error = NULL;
	GDBusConnection *conn;
	icDbusSkeleton *skeleton;

	DBG("SIG : %s, %s", signal_name, g_variant_print(value, FALSE));

	skeleton = IC_DBUS_SKELETON(icd_dbus_get_object());
	conn = g_dbus_interface_skeleton_get_connection(G_DBUS_INTERFACE_SKELETON(skeleton));

	ret = g_dbus_connection_emit_signal(conn,
			dest,
			IOTCON_DBUS_OBJPATH,
			IOTCON_DBUS_INTERFACE,
			signal_name,
			value,
			&error);
	if (FALSE == ret) {
		ERR("g_dbus_connection_emit_signal() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(value);
		return IOTCON_ERROR_DBUS;
	}

	if (FALSE == g_dbus_connection_flush_sync(conn, NULL, &error)) {
		ERR("g_dbus_connection_flush_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(value);
		return IOTCON_ERROR_DBUS;
	}

	return IOTCON_ERROR_NONE;
}

static void _icd_dbus_cleanup_handle(OCResourceHandle data)
{
	int ret;
	icd_resource_handle_s *rsrc_handle = data;

	RET_IF(NULL == rsrc_handle);
	RET_IF(NULL == rsrc_handle->handle);

	DBG("handle(%u, %u) deregistering", rsrc_handle->handle, rsrc_handle->signal_number);

	ret = icd_ioty_unregister_resource(rsrc_handle->handle);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unregister_resource() Fail(%d)", ret);

	free(rsrc_handle);
}

static int _icd_dbus_client_list_cleanup_handle_list(GList *client_list)
{
	icd_dbus_client_s *client;

	RETV_IF(NULL == client_list, IOTCON_ERROR_INVALID_PARAMETER);

	client = client_list->data;
	g_list_free_full(client->hdlist, _icd_dbus_cleanup_handle);
	free(client->bus_name);
	client->bus_name = NULL;
	free(client);
	g_list_free(client_list);

	return IOTCON_ERROR_NONE;
}

static int _icd_dbus_client_list_find_client(const gchar *owner, GList **ret_list)
{
	GList *client_list;
	icd_dbus_client_s *client;

	RETV_IF(NULL == owner, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ret_list, IOTCON_ERROR_INVALID_PARAMETER);

	client_list = icd_dbus_client_list;
	while (client_list) {
		client = client_list->data;
		if (NULL == client) {
			ERR("client is NULL");
			return IOTCON_ERROR_NO_DATA;
		}

		if (IC_STR_EQUAL == g_strcmp0(client->bus_name, owner)) {
			*ret_list = client_list;
			return IOTCON_ERROR_NONE;
		}

		client_list = client_list->next;
	}

	return IOTCON_ERROR_NONE;
}

static void _icd_dbus_name_owner_changed_cb(GDBusConnection *conn,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	int ret;
	GList *client = NULL;
	gchar *name, *old_owner, *new_owner;

	g_variant_get(parameters, "(&s&s&s)", &name, &old_owner, &new_owner);

	if (0 == strlen(new_owner)) {
		g_mutex_lock(&icd_dbus_client_list_mutex);
		ret = _icd_dbus_client_list_find_client(old_owner, &client);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_client_list_find_client() Fail(%d)", ret);
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return;
		}

		if (client) { /* found bus name in our bus list */
			DBG("bus(%s) stopped", old_owner);
			icd_dbus_client_list = g_list_remove_link(icd_dbus_client_list, client);
		}
		g_mutex_unlock(&icd_dbus_client_list_mutex);

		if (client) {
			ret = _icd_dbus_client_list_cleanup_handle_list(client);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icd_dbus_client_list_cleanup_handle_list() Fail(%d)", ret);
				return;
			}
		}
	}
}

static int _icd_dbus_subscribe_name_owner_changed(GDBusConnection *conn)
{
	FN_CALL;
	unsigned int id;

	id = g_dbus_connection_signal_subscribe(conn,
			"org.freedesktop.DBus", /* bus name */
			"org.freedesktop.DBus", /* interface */
			"NameOwnerChanged", /* member */
			"/org/freedesktop/DBus", /* path */
			NULL, /* arg0 */
			G_DBUS_SIGNAL_FLAGS_NONE,
			_icd_dbus_name_owner_changed_cb,
			NULL,
			NULL);
	if (0 == id) {
		ERR("g_dbus_connection_signal_subscribe() Fail");
		return IOTCON_ERROR_DBUS;
	}

	return IOTCON_ERROR_NONE;
}

static int _icd_dbus_resource_list_append_handle(const gchar *bus_name,
		OCResourceHandle handle, int64_t signal_number)
{
	FN_CALL;
	GList *cur_client, *cur_hd;
	bool client_exist = false;
	icd_resource_handle_s *rsrc_handle;
	icd_dbus_client_s *client = NULL;

	RETV_IF(NULL == bus_name, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == handle, IOTCON_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&icd_dbus_client_list_mutex);
	cur_client = icd_dbus_client_list;

	while (cur_client) {
		client = cur_client->data;
		if (NULL == client) {
			ERR("client is NULL");
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return IOTCON_ERROR_NO_DATA;
		}

		if (IC_STR_EQUAL == g_strcmp0(client->bus_name, bus_name)) {
			DBG("bus_name(%s) already exist", bus_name);
			client_exist = true;
			break;
		}

		cur_client = cur_client->next;
	}

	if (true == client_exist) {
		cur_hd = client->hdlist;
		while (cur_hd) {
			rsrc_handle = cur_hd->data;

			if (rsrc_handle->handle == handle) {
				ERR("resource handle(%u, %u) already exist", rsrc_handle->handle,
						rsrc_handle->signal_number);
				g_mutex_unlock(&icd_dbus_client_list_mutex);
				return IOTCON_ERROR_ALREADY;
			}
			cur_hd = cur_hd->next;
		}
	} else {
		DBG("bus_name(%s) not exist. make new one.", bus_name);

		client = calloc(1, sizeof(icd_dbus_client_s));
		if (NULL == client) {
			ERR("calloc(client) Fail(%d)", errno);
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		client->bus_name = ic_utils_strdup(bus_name);
		if (NULL == client->bus_name) {
			ERR("ic_utils_strdup() Fail");
			free(client);
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		DBG("new client(%s, %d) added", bus_name, signal_number);
	}

	rsrc_handle = calloc(1, sizeof(icd_resource_handle_s));
	if (NULL == rsrc_handle) {
		ERR("calloc(handle) Fail(%d)", errno);
		if (false == client_exist) {
			free(client->bus_name);
			free(client);
		}
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	rsrc_handle->handle = handle;
	rsrc_handle->signal_number = signal_number;

	DBG("handle(%u) added in the client(%s, %u)", handle, bus_name, signal_number);

	client->hdlist = g_list_append(client->hdlist, rsrc_handle);

	if (false == client_exist)
		icd_dbus_client_list = g_list_append(icd_dbus_client_list, client);

	g_mutex_unlock(&icd_dbus_client_list_mutex);
	return IOTCON_ERROR_NONE;
}

static gboolean _dbus_handle_register_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar* const *resource_types,
		gint ifaces,
		gint properties)
{
	FN_CALL;
	int ret;
	const gchar *sender;
	OCResourceHandle handle;
	int64_t signal_number = 0;

	handle = icd_ioty_register_resource(uri_path, resource_types, ifaces, properties);
	if (handle) {
		sender = g_dbus_method_invocation_get_sender(invocation);

		signal_number = icd_dbus_generate_signal_number();
		ret = _icd_dbus_resource_list_append_handle(sender, handle, signal_number);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_resource_list_append_handle() Fail(%d)", ret);

			ret = icd_ioty_unregister_resource(handle);
			if (IOTCON_ERROR_NONE != ret)
				ERR("icd_ioty_unregister_resource(%u) Fail(%d)", handle, ret);

			handle = NULL;
		}
	}

	ic_dbus_complete_register_resource(object, invocation, signal_number,
			ICD_POINTER_TO_INT64(handle));

	return TRUE;
}


static gboolean _dbus_handle_unregister_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 resource)
{
	int ret;

	ret = icd_ioty_unregister_resource(ICD_INT64_TO_POINTER(resource));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unregister_resource(%u) Fail(%d)", resource, ret);
	else
		DBG("handle(%u) deregistered", resource);

	_icd_dbus_resource_handle_free(ICD_INT64_TO_POINTER(resource));

	ic_dbus_complete_unregister_resource(object, invocation);

	return TRUE;
}


static gboolean _dbus_handle_bind_interface(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 resource, gint iface)
{
	int ret;

	ret = icd_ioty_bind_interface(ICD_INT64_TO_POINTER(resource), iface);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_interface() Fail(%d)", ret);

	ic_dbus_complete_bind_interface(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_type(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 resource, const gchar *type)
{
	int ret;

	ret = icd_ioty_bind_type(ICD_INT64_TO_POINTER(resource), type);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_type() Fail(%d)", ret);

	ic_dbus_complete_bind_type(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 parent, gint64 child)
{
	int ret;

	ret = icd_ioty_bind_resource(ICD_INT64_TO_POINTER(parent),
			ICD_INT64_TO_POINTER(child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_resource() Fail(%d)", ret);

	ic_dbus_complete_bind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_unbind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 parent, gint64 child)
{
	int ret;

	ret = icd_ioty_unbind_resource(ICD_INT64_TO_POINTER(parent),
			ICD_INT64_TO_POINTER(child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unbind_resource() Fail(%d)", ret);

	ic_dbus_complete_unbind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_find_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		gint connectivity,
		const gchar *type)
{
	int ret;
	const gchar *sender;
	int64_t signal_number;

	sender = g_dbus_method_invocation_get_sender(invocation);

	signal_number = icd_dbus_generate_signal_number();
	ret = icd_ioty_find_resource(host_address, connectivity, type, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_find_resource() Fail(%d)", ret);

	ic_dbus_complete_find_resource(object, invocation, signal_number, ret);

	return TRUE;
}


static gboolean _dbus_handle_observer_start(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *resource,
		gint observe_policy,
		GVariant *query)
{
	OCDoHandle observe_h;
	const gchar *sender;
	int64_t signal_number;

	signal_number = icd_dbus_generate_signal_number();
	sender = g_dbus_method_invocation_get_sender(invocation);

	observe_h = icd_ioty_observer_start(resource, observe_policy, query,
			signal_number, sender);
	if (NULL == observe_h)
		ERR("icd_ioty_observer_start() Fail");

	ic_dbus_complete_observer_start(object, invocation, signal_number,
			ICD_POINTER_TO_INT64(observe_h));

	/* observe_h will be freed in _dbus_handle_observer_stop() */
	return TRUE;
}


static gboolean _dbus_handle_observer_stop(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint64 observe_h,
		GVariant *options)
{
	int ret;

	ret = icd_ioty_observer_stop(ICD_INT64_TO_POINTER(observe_h), options);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_observer_stop() Fail(%d)", ret);

	ic_dbus_complete_observer_stop(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_notify(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint64 resource,
		GVariant *notify_msg,
		GVariant *observers)
{
	int ret;

	ret = icd_ioty_notify(ICD_INT64_TO_POINTER(resource), notify_msg, observers);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_notify() Fail(%d)", ret);

	ic_dbus_complete_notify(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_send_response(icDbus *object,
		GDBusMethodInvocation *invocation, GVariant *response)
{
	int ret;

	ret = icd_ioty_send_response(response);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_send_response() Fail(%d)", ret);

	ic_dbus_complete_send_response(object, invocation, ret);

	return TRUE;
}

static gboolean _dbus_handle_get_device_info(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		gint connectivity)
{
	int ret;
	const gchar *sender;
	int64_t signal_number;

	signal_number = icd_dbus_generate_signal_number();
	sender = g_dbus_method_invocation_get_sender(invocation);

	ret = icd_ioty_get_info(ICD_DEVICE_INFO, host_address, connectivity, signal_number,
			sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get_info(device info) Fail(%d)", ret);

	ic_dbus_complete_get_device_info(object, invocation, signal_number, ret);

	return TRUE;
}

static gboolean _dbus_handle_get_platform_info(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		gint connectivity)
{
	int ret;
	const gchar *sender;
	int64_t signal_number;

	signal_number = icd_dbus_generate_signal_number();
	sender = g_dbus_method_invocation_get_sender(invocation);

	ret = icd_ioty_get_info(ICD_PLATFORM_INFO, host_address, connectivity, signal_number,
			sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get_info(platform info) Fail(%d)", ret);

	ic_dbus_complete_get_platform_info(object, invocation, signal_number, ret);

	return TRUE;
}


static gboolean _dbus_handle_start_presence(icDbus *object,
		GDBusMethodInvocation *invocation,
		guint time_to_live)
{
	int ret;

	ret = icd_ioty_start_presence(time_to_live);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_start_presence() Fail(%d)", ret);

	ic_dbus_complete_start_presence(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_stop_presence(icDbus *object,
		GDBusMethodInvocation *invocation)
{
	int ret;

	ret = icd_ioty_stop_presence();
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_stop_presence() Fail(%d)", ret);

	ic_dbus_complete_stop_presence(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_subscribe_presence(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		gint connectivity,
		const gchar *type)
{
	OCDoHandle presence_h;

	presence_h = icd_ioty_subscribe_presence(ICD_PRESENCE, host_address, connectivity,
			type, NULL);
	if (NULL == presence_h)
		ERR("icd_ioty_subscribe_presence() Fail");

	ic_dbus_complete_subscribe_presence(object, invocation,
			ICD_POINTER_TO_INT64(presence_h));

	return TRUE;
}


static gboolean _dbus_handle_unsubscribe_presence(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint64 presence_h,
		const gchar *host_address)
{
	int ret;

	ret = icd_ioty_unsubscribe_presence(ICD_INT64_TO_POINTER(presence_h), host_address);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unsubscribe_presence() Fail(%d)", ret);

	ic_dbus_complete_unsubscribe_presence(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_encap_get_time_interval(icDbus *object,
		GDBusMethodInvocation *invocation)
{
	int time_interval;

	time_interval = icd_ioty_encap_get_time_interval();

	ic_dbus_complete_encap_get_time_interval(object, invocation, time_interval);

	return TRUE;
}


static gboolean _dbus_handle_encap_set_time_interval(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint time_interval)
{
	icd_ioty_encap_set_time_interval(time_interval);

	ic_dbus_complete_encap_set_time_interval(object, invocation);

	return TRUE;
}


static gboolean _dbus_handle_start_monitoring(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar *host_address,
		gint connectivity)
{
	int ret;
	int64_t signal_number;

	ret = icd_ioty_start_encap(ICD_ENCAP_MONITORING, uri_path, host_address,
			connectivity, &signal_number);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_start_encap() Fail(%d)", ret);

	ic_dbus_complete_start_monitoring(object, invocation, signal_number, ret);

	return TRUE;
}


static gboolean _dbus_handle_stop_monitoring(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar *host_address)
{
	int ret;

	ret = icd_ioty_stop_encap(ICD_ENCAP_MONITORING, uri_path, host_address);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_stop_encap() Fail(%d)", ret);

	ic_dbus_complete_stop_monitoring(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_start_caching(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar *host_address,
		gint connectivity)
{
	int ret;
	int64_t signal_number;

	ret = icd_ioty_start_encap(ICD_ENCAP_CACHING, uri_path, host_address,
			connectivity, &signal_number);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_start_encap() Fail(%d)", ret);

	ic_dbus_complete_start_monitoring(object, invocation, signal_number, ret);

	return TRUE;
}


static gboolean _dbus_handle_stop_caching(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar *host_address)
{
	int ret;

	ret = icd_ioty_stop_encap(ICD_ENCAP_CACHING, uri_path, host_address);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_stop_encap() Fail(%d)", ret);

	ic_dbus_complete_stop_monitoring(object, invocation, ret);

	return TRUE;
}


static void _dbus_on_bus_acquired(GDBusConnection *conn, const gchar *name,
		gpointer user_data)
{
	gboolean ret;
	GError *error = NULL;

	icd_dbus_object = ic_dbus_skeleton_new();
	if (NULL == icd_dbus_object) {
		ERR("ic_iotcon_skeletion_new() Fail");
		return;
	}

	g_signal_connect(icd_dbus_object, "handle-register-resource",
			G_CALLBACK(_dbus_handle_register_resource), NULL);
	g_signal_connect(icd_dbus_object, "handle-unregister-resource",
			G_CALLBACK(_dbus_handle_unregister_resource), NULL);
	g_signal_connect(icd_dbus_object, "handle-bind-interface",
			G_CALLBACK(_dbus_handle_bind_interface), NULL);
	g_signal_connect(icd_dbus_object, "handle-bind-type",
			G_CALLBACK(_dbus_handle_bind_type), NULL);
	g_signal_connect(icd_dbus_object, "handle-bind-resource",
			G_CALLBACK(_dbus_handle_bind_resource), NULL);
	g_signal_connect(icd_dbus_object, "handle-unbind-resource",
			G_CALLBACK(_dbus_handle_unbind_resource), NULL);
	g_signal_connect(icd_dbus_object, "handle-find-resource",
			G_CALLBACK(_dbus_handle_find_resource), NULL);
	g_signal_connect(icd_dbus_object, "handle-get",
			G_CALLBACK(icd_ioty_get), NULL);
	g_signal_connect(icd_dbus_object, "handle-put",
			G_CALLBACK(icd_ioty_put), NULL);
	g_signal_connect(icd_dbus_object, "handle-post",
			G_CALLBACK(icd_ioty_post), NULL);
	g_signal_connect(icd_dbus_object, "handle-delete",
			G_CALLBACK(icd_ioty_delete), NULL);
	g_signal_connect(icd_dbus_object, "handle-observer-start",
			G_CALLBACK(_dbus_handle_observer_start), NULL);
	g_signal_connect(icd_dbus_object, "handle-observer-stop",
			G_CALLBACK(_dbus_handle_observer_stop), NULL);
	g_signal_connect(icd_dbus_object, "handle-notify",
			G_CALLBACK(_dbus_handle_notify), NULL);
	g_signal_connect(icd_dbus_object, "handle-send-response",
			G_CALLBACK(_dbus_handle_send_response), NULL);
	g_signal_connect(icd_dbus_object, "handle-get-device-info",
			G_CALLBACK(_dbus_handle_get_device_info), NULL);
	g_signal_connect(icd_dbus_object, "handle-get-platform-info",
			G_CALLBACK(_dbus_handle_get_platform_info), NULL);
	g_signal_connect(icd_dbus_object, "handle-get-tizen-info",
			G_CALLBACK(icd_ioty_get_tizen_info), NULL);
	g_signal_connect(icd_dbus_object, "handle-start-presence",
			G_CALLBACK(_dbus_handle_start_presence), NULL);
	g_signal_connect(icd_dbus_object, "handle-stop-presence",
			G_CALLBACK(_dbus_handle_stop_presence), NULL);
	g_signal_connect(icd_dbus_object, "handle-subscribe-presence",
			G_CALLBACK(_dbus_handle_subscribe_presence), NULL);
	g_signal_connect(icd_dbus_object, "handle-unsubscribe-presence",
			G_CALLBACK(_dbus_handle_unsubscribe_presence), NULL);
	g_signal_connect(icd_dbus_object, "handle-encap-get-time-interval",
			G_CALLBACK(_dbus_handle_encap_get_time_interval), NULL);
	g_signal_connect(icd_dbus_object, "handle-encap-set-time-interval",
			G_CALLBACK(_dbus_handle_encap_set_time_interval), NULL);
	g_signal_connect(icd_dbus_object, "handle-start-monitoring",
			G_CALLBACK(_dbus_handle_start_monitoring), NULL);
	g_signal_connect(icd_dbus_object, "handle-stop-monitoring",
			G_CALLBACK(_dbus_handle_stop_monitoring), NULL);
	g_signal_connect(icd_dbus_object, "handle-start-caching",
			G_CALLBACK(_dbus_handle_start_caching), NULL);
	g_signal_connect(icd_dbus_object, "handle-stop-caching",
			G_CALLBACK(_dbus_handle_stop_caching), NULL);

	ret = g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(icd_dbus_object),
			conn, IOTCON_DBUS_OBJPATH, &error);
	if (FALSE == ret) {
		ERR("g_dbus_interface_skeleton_export() Fail(%s)", error->message);
		g_error_free(error);
	}

	ret = _icd_dbus_subscribe_name_owner_changed(conn);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_dbus_subscribe_name_owner_changed() Fail(%d)", ret);
		return;
	}

}


static void _dbus_on_name_lost(GDBusConnection *conn, const gchar *name,
		gpointer user_data)
{
	DBG("Lost the name %s", name);
}


static void _dbus_on_name_acquired(GDBusConnection *conn, const gchar *name,
		gpointer user_data)
{
	DBG("Acquired the name %s", name);
}


unsigned int icd_dbus_init()
{
	guint id;

	id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
			IOTCON_DBUS_INTERFACE,
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			_dbus_on_bus_acquired,
			_dbus_on_name_acquired,
			_dbus_on_name_lost,
			NULL,
			NULL);
	if (0 == id) {
		ERR("g_bus_own_name() Fail");
		return 0;
	}

	return id;
}


void icd_dbus_deinit(unsigned int id)
{
	g_bus_unown_name(id);
}

