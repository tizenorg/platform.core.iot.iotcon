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
#include "icd-cynara.h"
#include "icd-dbus.h"

static icDbus *icd_dbus_object;

/* global list to care resource handle for each client */
static GList *icd_dbus_client_list;
static GMutex icd_dbus_client_list_mutex;

typedef struct _icd_dbus_client_s {
	gchar *bus_name;
	GList *resource_list;
	GList *presence_list;
	GList *observe_list;
	GList *encap_list;
} icd_dbus_client_s;

typedef struct _icd_resource_handle {
	OCResourceHandle handle;
	int64_t signal_number;
} icd_resource_handle_s;

typedef struct _icd_presence_handle {
	OCDoHandle handle;
	char *host_address;
} icd_presence_handle_s;

typedef struct _icd_encap_handle {
	int type;
	char *host_address;
	char *uri_path;
} icd_encap_handle_s;

icDbus* icd_dbus_get_object()
{
	return icd_dbus_object;
}


int64_t icd_dbus_generate_signal_number()
{
	static int64_t i = 0;

	return i++;
}


int icd_dbus_client_list_get_resource_info(OCResourceHandle handle,
		int64_t *signal_number, gchar **bus_name)
{
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

		cur_hd = client->resource_list;
		while (cur_hd) {
			rsrc_handle = cur_hd->data;

			if (rsrc_handle->handle == handle) {
				DBG_HANDLE(handle);
				DBG("signal_number(%llx) found", rsrc_handle->signal_number);
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


static void _icd_dbus_cleanup_resource_list(void *data)
{
	int ret;
	icd_resource_handle_s *resource_handle = data;

	RET_IF(NULL == resource_handle);
	RET_IF(NULL == resource_handle->handle);

	DBG("Deregistering resource handle");
	DBG_HANDLE(resource_handle->handle);

	ret = icd_ioty_unregister_resource(resource_handle->handle);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unregister_resource() Fail(%d)", ret);

	free(resource_handle);
}


static void _icd_dbus_cleanup_presence_list(void *data)
{
	int ret;
	icd_presence_handle_s *presence_handle = data;

	RET_IF(NULL == presence_handle);
	RET_IF(NULL == presence_handle->handle);

	DBG("Deregistering presence handle");
	DBG_HANDLE(presence_handle->handle);

	ret = icd_ioty_unsubscribe_presence(presence_handle->handle,
			presence_handle->host_address);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unsubscribe_presence() Fail(%d)", ret);

	free(presence_handle->host_address);
	free(presence_handle);
}


static void _icd_dbus_cleanup_observe_list(OCDoHandle data)
{
	int ret;

	DBG("Deregistering observe handle");
	DBG_HANDLE(data);

	ret = icd_ioty_observer_stop(data, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_observer_stop() Fail(%d)", ret);
}


static void _icd_dbus_cleanup_encap_list(void *data)
{
	int ret;
	icd_encap_handle_s *encap_handle = data;

	RET_IF(NULL == encap_handle);

	DBG("Deregistering encapsulation");

	ret = icd_ioty_stop_encap(encap_handle->type, encap_handle->uri_path,
			encap_handle->host_address);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_stop_encap() Fail(%d)", ret);

	free(encap_handle->uri_path);
	free(encap_handle->host_address);
	free(encap_handle);
}


static int _icd_dbus_client_list_cleanup_handle_list(GList *client_list)
{
	FN_CALL;
	icd_dbus_client_s *client;

	RETV_IF(NULL == client_list, IOTCON_ERROR_INVALID_PARAMETER);

	client = client_list->data;

	/* resource list */
	g_list_free_full(client->resource_list, _icd_dbus_cleanup_resource_list);
	/* presence list */
	g_list_free_full(client->presence_list, _icd_dbus_cleanup_presence_list);
	/* observe list */
	g_list_free_full(client->observe_list, _icd_dbus_cleanup_observe_list);
	/* encapsulation list */
	g_list_free_full(client->encap_list, _icd_dbus_cleanup_encap_list);

	free(client->bus_name);
	client->bus_name = NULL;
	free(client);
	g_list_free(client_list);

	return IOTCON_ERROR_NONE;
}


static int _icd_dbus_client_list_compare_bus_name(const void *a, const void *b)
{
	const icd_dbus_client_s *client = a;

	return g_strcmp0(client->bus_name, b);
}


static inline GList* _icd_dbus_client_list_find_client(const gchar *owner)
{
	return g_list_find_custom(icd_dbus_client_list, owner,
			_icd_dbus_client_list_compare_bus_name);
}


static int _icd_dbus_client_list_get_client(const gchar *bus_name,
		icd_dbus_client_s **ret_client)
{
	GList *found_client = NULL;
	icd_dbus_client_s *client = NULL;

	RETV_IF(NULL == bus_name, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ret_client, IOTCON_ERROR_INVALID_PARAMETER);

	found_client = _icd_dbus_client_list_find_client(bus_name);
	if (found_client) {
		*ret_client = found_client->data;
		return IOTCON_ERROR_NONE;
	}

	client = calloc(1, sizeof(icd_dbus_client_s));
	if (NULL == client) {
		ERR("calloc(client) Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	client->bus_name = ic_utils_strdup(bus_name);
	if (NULL == client->bus_name) {
		ERR("ic_utils_strdup() Fail");
		free(client);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	DBG("new client(%s) added", bus_name);
	icd_dbus_client_list = g_list_append(icd_dbus_client_list, client);
	*ret_client = client;

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
		client = _icd_dbus_client_list_find_client(old_owner);
		if (client) { /* found bus name in our bus list */
			DBG("bus(%s) stopped", old_owner);
			icd_dbus_client_list = g_list_remove_link(icd_dbus_client_list, client);
		}
		g_mutex_unlock(&icd_dbus_client_list_mutex);

		if (client) {
			ret = _icd_dbus_client_list_cleanup_handle_list(client);
			if (IOTCON_ERROR_NONE != ret)
				ERR("_icd_dbus_client_list_cleanup_handle_list() Fail(%d)", ret);
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

static int _icd_dbus_resource_list_add(const gchar *bus_name, OCResourceHandle handle,
		int64_t signal_number)
{
	int ret;
	icd_dbus_client_s *client = NULL;
	icd_resource_handle_s *resource_handle;

	RETV_IF(NULL == bus_name, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == handle, IOTCON_ERROR_INVALID_PARAMETER);

	resource_handle = calloc(1, sizeof(icd_resource_handle_s));
	if (NULL == resource_handle) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	resource_handle->handle = handle;
	resource_handle->signal_number = signal_number;

	g_mutex_lock(&icd_dbus_client_list_mutex);
	ret = _icd_dbus_client_list_get_client(bus_name, &client);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_dbus_client_list_get_client() Fail");
		free(resource_handle);
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return ret;
	}

	DBG("Resource handle added in the client(%s)", bus_name);
	DBG_HANDLE(handle);

	client->resource_list = g_list_append(client->resource_list, resource_handle);

	g_mutex_unlock(&icd_dbus_client_list_mutex);
	return IOTCON_ERROR_NONE;
}


static void _icd_dbus_resource_list_remove(const gchar *bus_name, OCResourceHandle handle)
{
	GList *cur_hd;
	GList *client_list = NULL;
	icd_dbus_client_s *client = NULL;
	icd_resource_handle_s *resource_handle;

	g_mutex_lock(&icd_dbus_client_list_mutex);
	client_list = _icd_dbus_client_list_find_client(bus_name);
	if (NULL == client_list) {
		ERR("_icd_dbus_client_list_find_client() Fail");
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return;
	}

	client = client_list->data;
	cur_hd = client->resource_list;
	while (cur_hd) {
		resource_handle = cur_hd->data;

		if (resource_handle->handle == handle) {
			DBG("Resource handle is removed");
			DBG_HANDLE(handle);
			client->resource_list = g_list_delete_link(client->resource_list, cur_hd);
			free(resource_handle);
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return;
		}
		cur_hd = cur_hd->next;
	}
	g_mutex_unlock(&icd_dbus_client_list_mutex);
}


static int _icd_dbus_presence_list_add(const gchar *bus_name,
		OCDoHandle handle, const char *host_address)
{
	int ret;
	icd_dbus_client_s *client = NULL;
	icd_presence_handle_s *presence_handle;

	RETV_IF(NULL == bus_name, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == handle, IOTCON_ERROR_INVALID_PARAMETER);

	presence_handle = calloc(1, sizeof(icd_presence_handle_s));
	if (NULL == presence_handle) {
		ERR("calloc(handle) Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	presence_handle->handle = handle;
	presence_handle->host_address = ic_utils_strdup(host_address);

	g_mutex_lock(&icd_dbus_client_list_mutex);
	ret = _icd_dbus_client_list_get_client(bus_name, &client);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_dbus_client_list_get_client() Fail(%d)", ret);
		free(presence_handle->host_address);
		free(presence_handle);
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return ret;
	}

	DBG("Presence handle added in the client(%s)", bus_name);
	DBG_HANDLE(handle);

	client->presence_list = g_list_append(client->presence_list, presence_handle);

	g_mutex_unlock(&icd_dbus_client_list_mutex);
	return IOTCON_ERROR_NONE;
}


static void _icd_dbus_presence_list_remove(const gchar *bus_name,
		OCDoHandle handle)
{
	GList *cur_hd;
	GList *client_list = NULL;
	icd_dbus_client_s *client = NULL;
	icd_presence_handle_s *presence_handle;

	g_mutex_lock(&icd_dbus_client_list_mutex);
	client_list = _icd_dbus_client_list_find_client(bus_name);
	if (NULL == client_list) {
		ERR("_icd_dbus_client_list_find_client() Fail");
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return;
	}

	client = client_list->data;
	cur_hd = client->presence_list;
	while (cur_hd) {
		presence_handle = cur_hd->data;

		if (presence_handle->handle == handle) {
			DBG("Presence handle is removed");
			DBG_HANDLE(handle);
			client->presence_list = g_list_delete_link(client->presence_list, cur_hd);
			free(presence_handle->host_address);
			free(presence_handle);
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return;
		}
		cur_hd = cur_hd->next;
	}
	g_mutex_unlock(&icd_dbus_client_list_mutex);
}


static int _icd_dbus_observe_list_add(const gchar *bus_name, OCDoHandle handle)
{
	int ret;
	icd_dbus_client_s *client = NULL;

	RETV_IF(NULL == bus_name, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == handle, IOTCON_ERROR_INVALID_PARAMETER);

	g_mutex_lock(&icd_dbus_client_list_mutex);
	ret = _icd_dbus_client_list_get_client(bus_name, &client);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_dbus_client_list_get_client() Fail(%d)", ret);
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return ret;
	}

	DBG("Observe handle added in the client(%s)", bus_name);
	DBG_HANDLE(handle);

	client->observe_list = g_list_append(client->observe_list, handle);

	g_mutex_unlock(&icd_dbus_client_list_mutex);
	return IOTCON_ERROR_NONE;
}


static void _icd_dbus_observe_list_remove(const gchar *bus_name,
		OCDoHandle handle)
{
	GList *cur_hd;
	GList *client_list = NULL;
	OCDoHandle observe_handle;
	icd_dbus_client_s *client = NULL;

	g_mutex_lock(&icd_dbus_client_list_mutex);
	client_list = _icd_dbus_client_list_find_client(bus_name);
	if (NULL == client_list) {
		ERR("_icd_dbus_client_list_find_client() Fail");
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return;
	}

	client = client_list->data;
	cur_hd = client->observe_list;
	while (cur_hd) {
		observe_handle = cur_hd->data;

		if (observe_handle == handle) {
			DBG("Observe handle is removed");
			DBG_HANDLE(handle);
			client->observe_list = g_list_delete_link(client->observe_list, cur_hd);
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return;
		}
		cur_hd = cur_hd->next;
	}
	g_mutex_unlock(&icd_dbus_client_list_mutex);
}


static int _icd_dbus_encap_list_add(const gchar *bus_name, int type,
		const char *host_address, const char *uri_path)
{
	int ret;
	icd_dbus_client_s *client = NULL;
	icd_encap_handle_s *encap_handle;

	RETV_IF(NULL == bus_name, IOTCON_ERROR_INVALID_PARAMETER);

	encap_handle = calloc(1, sizeof(icd_encap_handle_s));
	if (NULL == encap_handle) {
		ERR("calloc(handle) Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	encap_handle->type = type;
	encap_handle->host_address = ic_utils_strdup(host_address);
	encap_handle->uri_path = ic_utils_strdup(uri_path);

	g_mutex_lock(&icd_dbus_client_list_mutex);
	ret = _icd_dbus_client_list_get_client(bus_name, &client);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_dbus_client_list_get_client() Fail(%d)", ret);
		free(encap_handle->uri_path);
		free(encap_handle->host_address);
		free(encap_handle);
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return ret;
	}

	DBG("encap info added in the client(%s)", bus_name);

	client->encap_list = g_list_append(client->encap_list, encap_handle);

	g_mutex_unlock(&icd_dbus_client_list_mutex);
	return IOTCON_ERROR_NONE;
}


static void _icd_dbus_encap_list_remove(const gchar *bus_name, int type,
		const char *host_address, const char *uri_path)
{
	GList *cur_hd;
	GList *client_list = NULL;
	icd_dbus_client_s *client = NULL;
	icd_encap_handle_s *encap_handle;

	g_mutex_lock(&icd_dbus_client_list_mutex);
	client_list = _icd_dbus_client_list_find_client(bus_name);
	if (NULL == client_list) {
		ERR("_icd_dbus_client_list_find_client() Fail");
		g_mutex_unlock(&icd_dbus_client_list_mutex);
		return;
	}

	client = client_list->data;
	cur_hd = client->encap_list;
	while (cur_hd) {
		encap_handle = cur_hd->data;

		if (type == encap_handle->type
				&& IC_STR_EQUAL == g_strcmp0(encap_handle->host_address, host_address)
				&& IC_STR_EQUAL == g_strcmp0(encap_handle->uri_path, uri_path)) {
			DBG("encap info(%s, %s) removed", host_address, uri_path);
			client->encap_list = g_list_delete_link(client->encap_list, cur_hd);
			free(encap_handle->uri_path);
			free(encap_handle->host_address);
			free(encap_handle);
			g_mutex_unlock(&icd_dbus_client_list_mutex);
			return;
		}
		cur_hd = cur_hd->next;
	}
	g_mutex_unlock(&icd_dbus_client_list_mutex);
}


static gboolean _dbus_handle_register_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar* const *resource_types,
		const gchar* const *ifaces,
		gint properties,
		gboolean is_lite)
{
	FN_CALL;
	int ret;
	const gchar *sender;
	OCResourceHandle handle;
	int64_t signal_number = 0;

	if (true == is_lite) {
		ret = icd_cynara_check_network(invocation);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icd_cynara_check_network() Fail(%d)", ret);
			ic_dbus_complete_register_resource(object, invocation, signal_number, ret);
			return TRUE;
		}
	}

	handle = icd_ioty_register_resource(uri_path, resource_types, ifaces, properties);
	if (handle) {
		sender = g_dbus_method_invocation_get_sender(invocation);

		signal_number = icd_dbus_generate_signal_number();
		ret = _icd_dbus_resource_list_add(sender, handle, signal_number);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_resource_list_add() Fail(%d)", ret);

			ret = icd_ioty_unregister_resource(handle);
			if (IOTCON_ERROR_NONE != ret)
				ERR("icd_ioty_unregister_resource() Fail(%d)", ret);

			handle = NULL;
		}
	} else {
		ERR("icd_ioty_register_resource() Fail");
	}

	ic_dbus_complete_register_resource(object, invocation, signal_number,
			IC_POINTER_TO_INT64(handle));

	return TRUE;
}


static gboolean _dbus_handle_unregister_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 resource)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	_icd_dbus_resource_list_remove(sender, IC_INT64_TO_POINTER(resource));

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_unregister_resource(object, invocation);
		return TRUE;
	}

	ret = icd_ioty_unregister_resource(IC_INT64_TO_POINTER(resource));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unregister_resource() Fail(%d)", ret);

	ic_dbus_complete_unregister_resource(object, invocation);

	return TRUE;
}


static gboolean _dbus_handle_bind_interface(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 resource, const gchar *iface)
{
	int ret;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_bind_interface(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_bind_interface(IC_INT64_TO_POINTER(resource), iface);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_interface() Fail(%d)", ret);

	ic_dbus_complete_bind_interface(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_type(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 resource, const gchar *type)
{
	int ret;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_bind_type(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_bind_type(IC_INT64_TO_POINTER(resource), type);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_type() Fail(%d)", ret);

	ic_dbus_complete_bind_type(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 parent, gint64 child)
{
	int ret;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_bind_resource(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_bind_resource(IC_INT64_TO_POINTER(parent),
			IC_INT64_TO_POINTER(child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_resource() Fail(%d)", ret);

	ic_dbus_complete_bind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_unbind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint64 parent, gint64 child)
{
	int ret;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_unbind_resource(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_unbind_resource(IC_INT64_TO_POINTER(parent),
			IC_INT64_TO_POINTER(child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unbind_resource() Fail(%d)", ret);

	ic_dbus_complete_unbind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_find_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		gint connectivity,
		const gchar *type,
		bool is_secure,
		gint timeout)
{
	int ret;
	const gchar *sender;
	int64_t signal_number;

	signal_number = icd_dbus_generate_signal_number();

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_find_resource(object, invocation, signal_number, ret);
		return TRUE;
	}

	sender = g_dbus_method_invocation_get_sender(invocation);

	ret = icd_ioty_find_resource(host_address, connectivity, type, is_secure, timeout,
			signal_number, sender);
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
	int ret;
	const gchar *sender;
	int64_t signal_number;
	OCDoHandle observe_h = 0;

	signal_number = icd_dbus_generate_signal_number();

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_observer_start(object, invocation, signal_number,
				IC_POINTER_TO_INT64(observe_h));
		return TRUE;
	}

	sender = g_dbus_method_invocation_get_sender(invocation);

	observe_h = icd_ioty_observer_start(resource, observe_policy, query,
			signal_number, sender);
	if (observe_h) {
		ret = _icd_dbus_observe_list_add(sender, observe_h);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_observe_list_add() Fail(%d)", ret);

			ret = icd_ioty_observer_stop(IC_INT64_TO_POINTER(observe_h), NULL);
			if (IOTCON_ERROR_NONE != ret)
				ERR("icd_ioty_observer_stop() fail(%d)", ret);

			observe_h = NULL;
		}
	} else {
		ERR("icd_ioty_observer_start() Fail");
	}

	ic_dbus_complete_observer_start(object, invocation, signal_number,
			IC_POINTER_TO_INT64(observe_h));

	/* observe_h will be freed in _dbus_handle_observer_stop() */
	return TRUE;
}


static gboolean _dbus_handle_observer_stop(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint64 observe_h,
		GVariant *options)
{
	int ret;
	const gchar *sender;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_observer_stop(object, invocation, ret);
		return TRUE;
	}

	sender = g_dbus_method_invocation_get_sender(invocation);
	_icd_dbus_observe_list_remove(sender, IC_INT64_TO_POINTER(observe_h));

	ret = icd_ioty_observer_stop(IC_INT64_TO_POINTER(observe_h), options);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_observer_stop() Fail(%d)", ret);

	ic_dbus_complete_observer_stop(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_notify(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint64 resource,
		GVariant *notify_msg,
		GVariant *observers,
		gint qos)
{
	int ret;

	/* iotcon_resource_notify()
	 * iotcon_lite_resource_update_state() */
	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_notify(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_notify(IC_INT64_TO_POINTER(resource), notify_msg, observers, qos);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_notify() Fail(%d)", ret);

	ic_dbus_complete_notify(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_send_response(icDbus *object,
		GDBusMethodInvocation *invocation, GVariant *response)
{
	int ret;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_send_response(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_send_response(response);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_send_response() Fail(%d)", ret);

	ic_dbus_complete_send_response(object, invocation, ret);

	return TRUE;
}

static gboolean _dbus_handle_get_device_info(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		gint connectivity,
		gint timeout)
{
	int ret;
	const gchar *sender;
	int64_t signal_number;

	signal_number = icd_dbus_generate_signal_number();

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_get_device_info(object, invocation, signal_number, ret);
		return TRUE;
	}

	sender = g_dbus_method_invocation_get_sender(invocation);

	ret = icd_ioty_get_info(ICD_DEVICE_INFO, host_address, connectivity, timeout,
			signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get_info(device info) Fail(%d)", ret);

	ic_dbus_complete_get_device_info(object, invocation, signal_number, ret);

	return TRUE;
}

static gboolean _dbus_handle_get_platform_info(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		gint connectivity,
		gint timeout)
{
	int ret;
	const gchar *sender;
	int64_t signal_number;

	signal_number = icd_dbus_generate_signal_number();

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_get_platform_info(object, invocation, signal_number, ret);
		return TRUE;
	}

	sender = g_dbus_method_invocation_get_sender(invocation);

	ret = icd_ioty_get_info(ICD_PLATFORM_INFO, host_address, connectivity, timeout,
			signal_number, sender);
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

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_start_presence(object, invocation, ret);
		return TRUE;
	}

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

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_start_presence(object, invocation, ret);
		return TRUE;
	}

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
	int ret;
	const gchar *sender;
	OCDoHandle presence_h = NULL;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_subscribe_presence(object, invocation,
				IC_POINTER_TO_INT64(presence_h));
		return TRUE;
	}

	presence_h = icd_ioty_subscribe_presence(ICD_PRESENCE, host_address, connectivity,
			type, NULL);
	if (presence_h) {
		sender = g_dbus_method_invocation_get_sender(invocation);

		ret = _icd_dbus_presence_list_add(sender, presence_h, host_address);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_presence_list_add() Fail(%d)", ret);

			ret = icd_ioty_unsubscribe_presence(presence_h, host_address);
			if (IOTCON_ERROR_NONE != ret)
				ERR("icd_ioty_unsubscribe_presence(%u) Fail(%d)", presence_h, ret);

			presence_h = NULL;
		}
	} else {
		ERR("icd_ioty_subscribe_presence() Fail");
	}

	ic_dbus_complete_subscribe_presence(object, invocation,
			IC_POINTER_TO_INT64(presence_h));

	return TRUE;
}


static gboolean _dbus_handle_unsubscribe_presence(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint64 presence_h,
		const gchar *host_address)
{
	int ret;
	const gchar *sender;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_unsubscribe_presence(object, invocation, ret);
		return TRUE;
	}

	sender = g_dbus_method_invocation_get_sender(invocation);
	_icd_dbus_presence_list_remove(sender, IC_INT64_TO_POINTER(presence_h));

	ret = icd_ioty_unsubscribe_presence(IC_INT64_TO_POINTER(presence_h), host_address);
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
	const gchar *sender;
	int64_t signal_number = 0;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_start_monitoring(object, invocation, signal_number, ret);
		return TRUE;
	}

	ret = icd_ioty_start_encap(ICD_ENCAP_MONITORING, uri_path, host_address, connectivity,
			&signal_number);
	if (IOTCON_ERROR_NONE == ret) {
		sender = g_dbus_method_invocation_get_sender(invocation);

		ret = _icd_dbus_encap_list_add(sender, ICD_ENCAP_MONITORING, host_address,
				uri_path);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_encap_list_add() Fail(%d)", ret);

			ret = icd_ioty_stop_encap(ICD_ENCAP_MONITORING, uri_path, host_address);
			if (IOTCON_ERROR_NONE != ret)
				ERR("icd_ioty_stop_encap() Fail(%d)", ret);

			signal_number = 0;
		}
	} else {
		ERR("icd_ioty_start_encap() Fail(%d)", ret);
	}

	ic_dbus_complete_start_monitoring(object, invocation, signal_number, ret);

	return TRUE;
}


static gboolean _dbus_handle_stop_monitoring(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar *host_address)
{
	int ret;
	const gchar *sender;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_stop_monitoring(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_stop_encap(ICD_ENCAP_MONITORING, uri_path, host_address);
	if (IOTCON_ERROR_NONE == ret) {
		sender = g_dbus_method_invocation_get_sender(invocation);
		_icd_dbus_encap_list_remove(sender, ICD_ENCAP_MONITORING, host_address, uri_path);
	} else {
		ERR("icd_ioty_stop_encap() Fail(%d)", ret);
	}

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
	const gchar *sender;
	int64_t signal_number = 0;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_start_monitoring(object, invocation, signal_number, ret);
		return TRUE;
	}

	ret = icd_ioty_start_encap(ICD_ENCAP_CACHING, uri_path, host_address, connectivity,
			&signal_number);
	if (IOTCON_ERROR_NONE == ret) {
		sender = g_dbus_method_invocation_get_sender(invocation);

		ret = _icd_dbus_encap_list_add(sender, ICD_ENCAP_CACHING, host_address, uri_path);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_encap_list_add() Fail(%d)", ret);

			ret = icd_ioty_stop_encap(ICD_ENCAP_CACHING, uri_path, host_address);
			if (IOTCON_ERROR_NONE != ret)
				ERR("icd_ioty_stop_encap() Fail(%d)", ret);

			signal_number = 0;
		}
	} else {
		ERR("icd_ioty_start_encap() Fail(%d)", ret);
	}

	ic_dbus_complete_start_caching(object, invocation, signal_number, ret);

	return TRUE;
}


static gboolean _dbus_handle_stop_caching(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar *host_address)
{
	int ret;
	const gchar *sender;

	ret = icd_cynara_check_network(invocation);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_check_network() Fail(%d)", ret);
		ic_dbus_complete_stop_monitoring(object, invocation, ret);
		return TRUE;
	}

	ret = icd_ioty_stop_encap(ICD_ENCAP_CACHING, uri_path, host_address);
	if (IOTCON_ERROR_NONE == ret) {
		sender = g_dbus_method_invocation_get_sender(invocation);
		_icd_dbus_encap_list_remove(sender, ICD_ENCAP_CACHING, host_address, uri_path);
	} else {
		ERR("icd_ioty_stop_encap() Fail(%d)", ret);
	}

	ic_dbus_complete_stop_caching(object, invocation, ret);

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

