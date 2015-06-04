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

#include "icd.h"
#include "icd-dbus.h"

static GDBusConnection *icd_dbus_conn;

int icd_dbus_publish(const char *key, const char *data)
{
	gboolean ret;
	GError *error = NULL;
	int res = 0;
	char *path = NULL;

	ret = g_dbus_connection_emit_signal(icd_dbus_conn,
			NULL, /* destination bus name */
			"/",
			IOTCON_DBUS_INTERFACE,
			key,
			g_variant_new("(s)", data),
			&error);
	if (FALSE == ret)
	{
		ERR("g_dbus_connection_emit_signal() Fail(%s)", error->message);
		g_error_free(error);
		return -1;
	}

	if (FALSE == g_dbus_connection_flush_sync(icd_dbus_conn, NULL, &error))
	{
		ERR("g_dbus_connection_flush_sync() Fail(%s)", error->message);
		g_error_free(error);
	}

	if (path)
		free(path);

	return 0;
}

static void _dbus_handle_method_call(GDBusConnection *connection,
		const gchar *sender,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *method_name,
		GVariant *parameters,
		GDBusMethodInvocation *invocation,
		gpointer user_data)
{
	const gchar *key = NULL;

	if (0 == g_strcmp0(method_name, IOTCON_DBUS_METHOD1)) {
		g_variant_get(parameters, "(&s)", &key);
		if (NULL == key) {
			ERR("key is NULL");
			// TODO: handle error
		}

		//icd_handler_subscribe(key);

		// TODO: handle disconnect without unsubscribe

		g_dbus_method_invocation_return_value(invocation, NULL);
	}
	else if (0 == g_strcmp0(method_name, IOTCON_DBUS_METHOD2)) {
		g_variant_get(parameters, "(&s)", &key);
		if (NULL == key) {
			ERR("key is NULL");
			// TODO: handle error
		}

		//icd_handler_unsubscribe(key);

		g_dbus_method_invocation_return_value(invocation, NULL);
	}
	else if (0 == g_strcmp0(method_name, IOTCON_DBUS_METHOD3)) {
		const gchar *data = NULL;

		g_variant_get(parameters, "(&s&s)", &key, &data);
		if (NULL == key) {
			ERR("key is NULL");
			// TODO: handle error
		}

		//icd_handler_publish(key, data);

		g_dbus_method_invocation_return_value(invocation, NULL);
	}
	else {
		g_dbus_method_invocation_return_error(invocation,
			G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
			"Method %s is not implemented on interface %s", method_name, interface_name);
	}
}


static const GDBusInterfaceVTable interface_vtable =
{
	_dbus_handle_method_call,
	NULL,
	NULL
};


static void _dbus_on_bus_acquired(GDBusConnection *conn, const gchar *name,
		gpointer user_data)
{
	guint registration_id;
	GError *error = NULL;
	GDBusNodeInfo *introspection_data = user_data;

	FN_CALL;

	icd_dbus_conn = conn;

	registration_id = g_dbus_connection_register_object(conn,
		IOTCON_DBUS_OBJPATH,
		introspection_data->interfaces[0],
		&interface_vtable,
		NULL,/* user_data */
		NULL,/* user_data_free_func */
		&error);
	if (0 == registration_id)
	{
		ERR("g_dbus_connection_register_object() Fail(%s)", error->message);
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
	GError *error = NULL;
	GDBusNodeInfo *introspection_data = NULL;

	const gchar introspection_xml[] =
	"<node>"
	"	<interface name='"IOTCON_DBUS_INTERFACE"'>"
	"		<method name='"IOTCON_DBUS_METHOD1"'>"
	"			<arg type='s' name='key' direction='in'/>"
	"		</method>"
	"		<method name='"IOTCON_DBUS_METHOD2"'>"
	"			<arg type='s' name='key' direction='in'/>"
	"		</method>"
	"		<method name='"IOTCON_DBUS_METHOD3"'>"
	"			<arg type='s' name='key' direction='in'/>"
	"			<arg type='s' name='data' direction='in'/>"
	"		</method>"
	"	</interface>"
	"</node>";

	introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, &error);
	if (NULL == introspection_data)
	{
		ERR("g_dbus_node_info_new_for_xml() Fail(%s)", error->message);
		g_error_free(error);
		return -1;
	}

	id = g_bus_own_name(G_BUS_TYPE_SYSTEM,
			IOTCON_DBUS_INTERFACE,
			G_BUS_NAME_OWNER_FLAGS_REPLACE,
			_dbus_on_bus_acquired,
			_dbus_on_name_acquired,
			_dbus_on_name_lost,
			introspection_data,
			(GDestroyNotify)g_dbus_node_info_unref);
	if (0 == id)
	{
		ERR("g_bus_own_name() Fail");
		return 0;
	}

	return id;
}

void icd_dbus_deinit(unsigned int id)
{
	g_bus_unown_name(id);
}

