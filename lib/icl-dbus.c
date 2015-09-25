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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gio/gio.h>

#include "iotcon.h"
#include "ic-common.h"
#include "ic-dbus.h"
#include "icl.h"
#include "icl-dbus.h"

static unsigned int icl_dbus_count;
static icDbus *icl_dbus_object;
static GList *icl_dbus_sub_ids;
static GList *icl_dbus_conn_changed_cbs;

typedef struct {
	void *cb;
	void *user_data;
	unsigned int id;
} icl_cb_container_s;


icDbus* icl_dbus_get_object()
{
	return icl_dbus_object;
}


inline unsigned int icl_dbus_generate_signal_number()
{
	static unsigned int i = 0;

	return i++;
}


unsigned int icl_dbus_subscribe_signal(char *sig_name, void *cb_container, void *cb_free,
		GDBusSignalCallback sig_handler)
{
	unsigned int id;
	GDBusConnection *conn = g_dbus_proxy_get_connection(G_DBUS_PROXY(icl_dbus_object));

	id = g_dbus_connection_signal_subscribe(conn,
			NULL,
			IOTCON_DBUS_INTERFACE,
			sig_name,
			IOTCON_DBUS_OBJPATH,
			NULL,
			G_DBUS_SIGNAL_FLAGS_NONE,
			sig_handler,
			cb_container,
			cb_free);
	if (0 == id) {
		ERR("g_dbus_connection_signal_subscribe() Fail");
		return id;
	}

	icl_dbus_sub_ids = g_list_append(icl_dbus_sub_ids, GUINT_TO_POINTER(id));

	return id;
}


void icl_dbus_unsubscribe_signal(unsigned int id)
{
	GDBusConnection *conn = g_dbus_proxy_get_connection(G_DBUS_PROXY(icl_dbus_object));

	g_dbus_connection_signal_unsubscribe(conn, id);

	icl_dbus_sub_ids = g_list_remove(icl_dbus_sub_ids, GUINT_TO_POINTER(id));
}


static void _icl_dbus_connection_changed_cb(GObject *object, GParamSpec *pspec,
		gpointer user_data)
{
	bool is_connected = false;
	GDBusProxy *proxy = G_DBUS_PROXY(object);
	gchar *name_owner = g_dbus_proxy_get_name_owner(proxy);
	icl_cb_container_s *cb_container = user_data;
	iotcon_connection_changed_cb cb = cb_container->cb;

	if (name_owner)
		is_connected = true;

	if (cb)
		cb(is_connected, cb_container->user_data);
}


static icl_cb_container_s* _dbus_find_conn_changed_cb(iotcon_connection_changed_cb cb,
		void *user_data)
{
	GList *node;

	for (node = icl_dbus_conn_changed_cbs; node; node = node->next) {
		icl_cb_container_s *cb_container = node->data;
		if ((cb == cb_container->cb) && (user_data == cb_container->user_data))
			return cb_container;
	}

	return NULL;
}


API int iotcon_add_connection_changed_cb(iotcon_connection_changed_cb cb, void *user_data)
{
	unsigned int id;

	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	icl_cb_container_s *cb_container;

	if (_dbus_find_conn_changed_cb(cb, user_data)) {
		ERR("This callback is already registered.");
		return IOTCON_ERROR_ALREADY;
	}

	cb_container = calloc(1, sizeof(icl_cb_container_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	id = g_signal_connect_after(icl_dbus_object, "notify::g-name-owner",
			G_CALLBACK(_icl_dbus_connection_changed_cb), cb_container);
	if (0 == id) {
		ERR("g_signal_connect_after() Fail");
		free(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = id;

	icl_dbus_conn_changed_cbs = g_list_append(icl_dbus_conn_changed_cbs, cb_container);

	return IOTCON_ERROR_NONE;
}


API int iotcon_remove_connection_changed_cb(iotcon_connection_changed_cb cb,
		void *user_data)
{
	icl_cb_container_s *cb_container;

	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = _dbus_find_conn_changed_cb(cb, user_data);
	if (NULL == cb_container) {
		ERR("This callback is not registered");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	icl_dbus_conn_changed_cbs = g_list_remove(icl_dbus_conn_changed_cbs, cb_container);

	g_signal_handler_disconnect(icl_dbus_object, cb_container->id);
	free(cb_container);

	return IOTCON_ERROR_NONE;
}


static void _icl_dbus_sub_id_list_free(gpointer data)
{
	GDBusConnection *conn = g_dbus_proxy_get_connection(G_DBUS_PROXY(icl_dbus_object));

	g_dbus_connection_signal_unsubscribe(conn, GPOINTER_TO_UINT(data));
}


/* Unsubscribe all signals */
static void _icl_dbus_cleanup()
{
	g_list_free_full(icl_dbus_sub_ids, _icl_dbus_sub_id_list_free);
	icl_dbus_sub_ids = NULL;
}


static void _icl_dbus_name_owner_notify(GObject *object, GParamSpec *pspec,
		gpointer user_data)
{
	GDBusProxy *proxy = G_DBUS_PROXY(object);
	gchar *name_owner = g_dbus_proxy_get_name_owner(proxy);

	if (name_owner)
		return;

	_icl_dbus_cleanup();
}


inline int icl_dbus_convert_daemon_error(int error)
{
	int ret;

	if (IOTCON_ERROR_INVALID_PARAMETER == error)
		ret = IOTCON_ERROR_SYSTEM;
	else
		ret = error;

	return ret;
}


inline int icl_dbus_convert_dbus_error(int error)
{
	int ret;

	if (G_DBUS_ERROR_ACCESS_DENIED == error)
		ret = IOTCON_ERROR_PERMISSION_DENIED;
	else
		ret = IOTCON_ERROR_DBUS;

	return ret;
}


int icl_dbus_start()
{
	unsigned int id;
	GError *error = NULL;

	if (icl_dbus_object) {
		icl_dbus_count++;
		return IOTCON_ERROR_NONE;
	}

	icl_dbus_object = ic_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
			G_DBUS_PROXY_FLAGS_NONE,
			IOTCON_DBUS_INTERFACE,
			IOTCON_DBUS_OBJPATH,
			NULL,
			&error);
	if (NULL == icl_dbus_object) {
		ERR("ic_dbus_proxy_new_for_bus_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	id = g_signal_connect(icl_dbus_object, "notify::g-name-owner",
			G_CALLBACK(_icl_dbus_name_owner_notify), NULL);
	if (0 == id) {
		ERR("g_signal_connect() Fail");
		return IOTCON_ERROR_DBUS;
	}

	icl_dbus_count++;
	return IOTCON_ERROR_NONE;
}


int icl_dbus_stop()
{
	if (0 == icl_dbus_count) {
		ERR("dbus not initialized");
		return IOTCON_ERROR_DBUS;
	}

	icl_dbus_count--;

	if (0 < icl_dbus_count)
		return IOTCON_ERROR_NONE;

	DBG("All connection is closed");

	_icl_dbus_cleanup();

	g_list_free_full(icl_dbus_conn_changed_cbs, free);
	icl_dbus_conn_changed_cbs = NULL;

	g_object_unref(icl_dbus_object);
	icl_dbus_object = NULL;

	return IOTCON_ERROR_NONE;
}
