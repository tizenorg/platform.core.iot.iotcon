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
#include <stdlib.h>
#include <gio/gio.h>

#include "iotcon.h"
#include "ic-common.h"
#include "ic-dbus.h"
#include "icd.h"
#include "icd-ioty.h"
#include "icd-dbus.h"

static GDBusConnection *icd_dbus_conn;
static icDbus *icd_dbus_object;

int icd_dbus_emit_signal(const char *sig_name, GVariant *value)
{
	gboolean ret;
	GError *error = NULL;

	DBG("SIG : %s, %s", sig_name, g_variant_print(value, FALSE));

	ret = g_dbus_connection_emit_signal(icd_dbus_conn,
			NULL,
			IOTCON_DBUS_OBJPATH,
			IOTCON_DBUS_INTERFACE,
			sig_name,
			value,
			&error);
	if (FALSE == ret) {
		ERR("g_dbus_connection_emit_signal() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (FALSE == g_dbus_connection_flush_sync(icd_dbus_conn, NULL, &error)) {
		ERR("g_dbus_connection_flush_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return IOTCON_ERROR_NONE;
}


static gboolean _dbus_handle_register_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *arg_uri,
		const gchar* const *arg_resource_types,
		gint arg_ifaces,
		guchar arg_properties)
{
	void *handle = NULL;

	handle = icd_ioty_register_resource(arg_uri, arg_resource_types, arg_ifaces,
			arg_properties);
	if (NULL == handle)
		ERR("icd_ioty_register_resource() Fail");

	ic_dbus_complete_register_resource(object, invocation, GPOINTER_TO_INT(handle));

	return TRUE;
}


static gboolean _dbus_handle_unregister_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint arg_resource)
{
	int ret;

	ret = icd_ioty_unregister_resource(GINT_TO_POINTER(arg_resource));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unregister_resource() Fail(%d)", ret);

	ic_dbus_complete_unregister_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_interface(icDbus *object,
		GDBusMethodInvocation *invocation, gint arg_resource, gint arg_iface)
{
	int ret;

	ret = icd_ioty_bind_interface(GINT_TO_POINTER(arg_resource), arg_iface);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_interface() Fail(%d)", ret);

	ic_dbus_complete_bind_interface(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_type(icDbus *object,
		GDBusMethodInvocation *invocation, gint arg_resource, const gchar *arg_type)
{
	int ret;

	ret = icd_ioty_bind_type(GINT_TO_POINTER(arg_resource), arg_type);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_type() Fail(%d)", ret);

	ic_dbus_complete_bind_type(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint arg_parent, gint arg_child)
{
	int ret;

	ret = icd_ioty_bind_resource(GINT_TO_POINTER(arg_parent), GINT_TO_POINTER(arg_child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_resource() Fail(%d)", ret);

	ic_dbus_complete_bind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_unbind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint arg_parent, gint arg_child)
{
	int ret;

	ret = icd_ioty_unbind_resource(GINT_TO_POINTER(arg_parent),
			GINT_TO_POINTER(arg_child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unbind_resource() Fail(%d)", ret);

	ic_dbus_complete_unbind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_find_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *arg_host_address,
		const gchar *arg_type,
		const gchar *sig_name)
{
	int ret;

	ret = icd_ioty_find_resource(arg_host_address, arg_type, sig_name);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_find_resource() Fail(%d)", ret);

	ic_dbus_complete_find_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_get(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *arg_client, GVariant *arg_query, const gchar *sig_name)
{
	int ret;

	ret = icd_ioty_get(arg_client, arg_query, sig_name);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get() Fail(%d)", ret);

	ic_dbus_complete_get(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_put(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *arg_client,
		const gchar *arg_repr,
		GVariant *arg_query,
		const gchar *sig_name)
{
	int ret;

	ret = icd_ioty_put(arg_client, arg_repr, arg_query, sig_name);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_put() Fail(%d)", ret);

	ic_dbus_complete_put(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_post(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *arg_client,
		const gchar *arg_repr,
		GVariant *arg_query,
		const gchar *sig_name)
{
	int ret;

	ret = icd_ioty_post(arg_client, arg_repr, arg_query, sig_name);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_post() Fail(%d)", ret);

	ic_dbus_complete_post(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_delete(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *arg_client,
		const gchar *sig_name)
{
	int ret;

	ret = icd_ioty_delete(arg_client, sig_name);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_delete() Fail(%d)", ret);

	ic_dbus_complete_delete(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_observer_start(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *arg_client,
		gint arg_observe_type,
		GVariant *arg_query,
		const gchar *sig_name)
{
	int ret;
	int observe_h;

	ret = icd_ioty_observer_start(arg_client, arg_observe_type, arg_query, sig_name,
			&observe_h);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_observer_start() Fail(%d)", ret);

	ic_dbus_complete_observer_start(object, invocation, observe_h, ret);

	return TRUE;
}


static gboolean _dbus_handle_observer_stop(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint arg_observe_h)
{
	int ret;

	ret = icd_ioty_observer_stop(GINT_TO_POINTER(arg_observe_h));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_observer_stop() Fail(%d)", ret);

	ic_dbus_complete_observer_stop(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_notify_list_of_observers(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint arg_resource,
		GVariant *arg_notify_msg,
		GVariant *arg_observers)
{
	int ret;

	ret = icd_ioty_notify_list_of_observers(arg_resource, arg_notify_msg, arg_observers);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_notify_list_of_observers() Fail(%d)", ret);

	ic_dbus_complete_notify_list_of_observers(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_notify_all(icDbus *object, GDBusMethodInvocation *invocation,
		gint arg_resource)
{
	int ret;

	ret = icd_ioty_notify_all(arg_resource);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_notify_all() Fail(%d)", ret);

	ic_dbus_complete_notify_all(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_send_response(icDbus *object,
		GDBusMethodInvocation *invocation, GVariant *arg_response)
{
	int ret;

	ret = icd_ioty_send_response(arg_response);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_send_response() Fail(%d)", ret);

	ic_dbus_complete_send_response(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_register_device_info(icDbus *object,
		GDBusMethodInvocation *invocation, GVariant *arg_device_info)
{
	int ret;

	ret = icd_ioty_register_device_info(arg_device_info);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_register_device_info() Fail(%d)", ret);

	ic_dbus_complete_register_device_info(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_get_device_info(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *arg_host_address,
		const gchar *sig_name)
{
	int ret;

	ret = icd_ioty_get_device_info(arg_host_address, sig_name);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get_device_info() Fail(%d)", ret);

	ic_dbus_complete_get_device_info(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_start_presence(icDbus *object,
		GDBusMethodInvocation *invocation,
		guint arg_time_to_live)
{
	int ret;

	ret = icd_ioty_start_presence(arg_time_to_live);
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
		const gchar *arg_host_address,
		const gchar *arg_type,
		const gchar *sig_name)
{
	void *presence_h;

	presence_h = icd_ioty_subscribe_presence(arg_host_address, arg_type, sig_name);
	if (NULL == presence_h)
		ERR("icd_ioty_subscribe_presence() Fail");

	ic_dbus_complete_subscribe_presence(object, invocation,
			GPOINTER_TO_INT(presence_h));

	return TRUE;
}


static gboolean _dbus_handle_unsubscribe_presence(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint arg_presence_h)
{
	int ret;

	ret = icd_ioty_unsubscribe_presence(GINT_TO_POINTER(arg_presence_h));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unsubscribe_presence() Fail(%d)", ret);

	ic_dbus_complete_unsubscribe_presence(object, invocation, ret);

	return TRUE;
}


static void _dbus_on_bus_acquired(GDBusConnection *conn, const gchar *name,
		gpointer user_data)
{
	GError *error = NULL;
	gboolean ret;

	icd_dbus_conn = conn;

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
			G_CALLBACK(_dbus_handle_get), NULL);
	g_signal_connect(icd_dbus_object, "handle-put",
			G_CALLBACK(_dbus_handle_put), NULL);
	g_signal_connect(icd_dbus_object, "handle-post",
			G_CALLBACK(_dbus_handle_post), NULL);
	g_signal_connect(icd_dbus_object, "handle-delete",
			G_CALLBACK(_dbus_handle_delete), NULL);
	g_signal_connect(icd_dbus_object, "handle-observer-start",
			G_CALLBACK(_dbus_handle_observer_start), NULL);
	g_signal_connect(icd_dbus_object, "handle-observer-stop",
			G_CALLBACK(_dbus_handle_observer_stop), NULL);
	g_signal_connect(icd_dbus_object, "handle-notify-list-of-observers",
			G_CALLBACK(_dbus_handle_notify_list_of_observers), NULL);
	g_signal_connect(icd_dbus_object, "handle-notify-all",
			G_CALLBACK(_dbus_handle_notify_all), NULL);
	g_signal_connect(icd_dbus_object, "handle-send-response",
			G_CALLBACK(_dbus_handle_send_response), NULL);
	g_signal_connect(icd_dbus_object, "handle-register-device-info",
			G_CALLBACK(_dbus_handle_register_device_info), NULL);
	g_signal_connect(icd_dbus_object, "handle-get-device-info",
			G_CALLBACK(_dbus_handle_get_device_info), NULL);
	g_signal_connect(icd_dbus_object, "handle-start-presence",
			G_CALLBACK(_dbus_handle_start_presence), NULL);
	g_signal_connect(icd_dbus_object, "handle-stop-presence",
			G_CALLBACK(_dbus_handle_stop_presence), NULL);
	g_signal_connect(icd_dbus_object, "handle-subscribe-presence",
			G_CALLBACK(_dbus_handle_subscribe_presence), NULL);
	g_signal_connect(icd_dbus_object, "handle-unsubscribe-presence",
			G_CALLBACK(_dbus_handle_unsubscribe_presence), NULL);

	ret = g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(icd_dbus_object), conn,
			IOTCON_DBUS_OBJPATH, &error);
	if (FALSE == ret) {
		ERR("g_dbus_interface_skeleton_export() Fail(%s)", error->message);
		g_error_free(error);
	}
}


static void _dbus_on_name_lost(GDBusConnection *connection, const gchar *name,
		gpointer user_data)
{
	DBG("Lost the name %s", name);
}


static void _dbus_on_name_acquired(GDBusConnection *connection, const gchar *name,
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

