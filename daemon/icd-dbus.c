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
#include "ic-utils.h"
#include "ic-dbus.h"
#include "icd.h"
#include "icd-ioty.h"
#include "icd-dbus.h"

static GDBusConnection *icd_dbus_conn;
static GList *icd_bus_list; /* global list to care resource handle for each sender bus */

typedef struct _icd_bus_s {
	gchar *sender;
	GList *hdlist;
} icd_bus_s;

typedef struct _ic_resource_handle {
	void *handle;
	unsigned int number;
} icd_resource_handle_s;

static void _icd_dbus_resource_handle_free(int handle)
{
	icd_bus_s *bus;
	GList *cur_bus, *cur_hd;
	icd_resource_handle_s *rsrc_handle;

	cur_bus = icd_bus_list;
	while (cur_bus) {
		bus = cur_bus->data;
		if (NULL == bus) {
			ERR("bus is NULL");
			return;
		}

		cur_hd = bus->hdlist;
		while (cur_hd) {
			rsrc_handle = cur_hd->data;

			if (rsrc_handle->handle == GINT_TO_POINTER(handle)) {
				DBG("resource handle(%u, %u) removed from handle list", handle, rsrc_handle->number);
				bus->hdlist = g_list_delete_link(bus->hdlist, cur_hd);
				free(rsrc_handle);
				return;
			}
			cur_hd = cur_hd->next;
		}

		cur_bus = cur_bus->next;
	}

	return;
}

int icd_dbus_bus_list_get_info(int handle, unsigned int *sig_num, const gchar **sender)
{
	FN_CALL;
	icd_bus_s *bus;
	GList *cur_bus, *cur_hd;
	icd_resource_handle_s *rsrc_handle;

	RETV_IF(NULL == sig_num, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == sender, IOTCON_ERROR_INVALID_PARAMETER);

	cur_bus = icd_bus_list;
	while (cur_bus) {
		bus = cur_bus->data;
		if (NULL == bus) {
			ERR("bus is NULL");
			return IOTCON_ERROR_NO_DATA;
		}

		cur_hd = bus->hdlist;
		while (cur_hd) {
			rsrc_handle = cur_hd->data;

			if (rsrc_handle->handle == GINT_TO_POINTER(handle)) {
				DBG("signal_number(%u) for resource handle(%u) found", rsrc_handle->number, handle);
				*sig_num = rsrc_handle->number;
				*sender = bus->sender;
				return IOTCON_ERROR_NONE;
			}
			cur_hd = cur_hd->next;
		}

		cur_bus = cur_bus->next;
	}

	return IOTCON_ERROR_NO_DATA;
}

int icd_dbus_emit_signal(const char *dest, const char *signal_name, GVariant *value)
{
	gboolean ret;
	GError *error = NULL;

	DBG("SIG : %s, %s", signal_name, g_variant_print(value, FALSE));

	ret = g_dbus_connection_emit_signal(icd_dbus_conn,
			dest,
			IOTCON_DBUS_OBJPATH,
			IOTCON_DBUS_INTERFACE,
			signal_name,
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

void _ic_dbus_cleanup_handle(icd_resource_handle_s *rsrc_handle)
{
	int ret;

	RET_IF(NULL == rsrc_handle);
	RET_IF(NULL == rsrc_handle->handle);

	DBG("handle(%u, %u) deregistering", rsrc_handle->handle, rsrc_handle->number);

	ret = icd_ioty_unregister_resource(rsrc_handle->handle);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unregister_resource() Fail(%d)", ret);

	free(rsrc_handle);
}

static int _icd_dbus_bus_list_cleanup_handle_list(GList *list)
{
	icd_bus_s *bus;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	bus = list->data;
	g_list_free_full(bus->hdlist, (GDestroyNotify)_ic_dbus_cleanup_handle);
	free(bus->sender);
	bus->sender = NULL;
	free(bus);

	return IOTCON_ERROR_NONE;
}

static int _icd_dbus_bus_list_find_sender(const gchar *owner, GList **ret_list)
{
	GList *cur;
	icd_bus_s *bus;

	RETV_IF(NULL == owner, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ret_list, IOTCON_ERROR_INVALID_PARAMETER);

	cur = icd_bus_list;
	while (cur) {
		bus = cur->data;
		if (NULL == bus) {
			ERR("bus is NULL");
			return IOTCON_ERROR_NO_DATA;
		}

		if (IC_STR_EQUAL == g_strcmp0(bus->sender, owner)) {
			*ret_list = cur;
			return IOTCON_ERROR_NONE;
		}

		cur = cur->next;
	}

	return IOTCON_ERROR_NONE;
}

static void _icd_dbus_name_owner_changed_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	int ret;
	GList *sender = NULL;
	gchar *name, *old_owner, *new_owner;

	g_variant_get(parameters, "(&s&s&s)", &name, &old_owner, &new_owner);

	if (0 == strlen(new_owner)) {
		ret = _icd_dbus_bus_list_find_sender(old_owner, &sender);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_bus_list_find_sender() Fail(%d)", ret);
			return;
		}

		if (sender) { /* found bus name in our bus list */
			DBG("bus(%s) stopped", old_owner);

			ret = _icd_dbus_bus_list_cleanup_handle_list(sender);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icd_dbus_bus_list_cleanup_handle_list() Fail(%d)", ret);
				return;
			}
			icd_bus_list = g_list_delete_link(icd_bus_list, sender);
		}
	}
}

static int _icd_dbus_subscribe_name_owner_changed()
{
	FN_CALL;
	unsigned int id;

	id = g_dbus_connection_signal_subscribe(icd_dbus_conn,
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

static int _icd_dbus_resource_list_append_handle(const gchar *sender, void *handle,
		unsigned int signal_number)
{
	FN_CALL;
	char *sender_dup;
	GList *cur_bus, *cur_hd;
	bool sender_exist = false;
	icd_resource_handle_s *rsrc_handle;
	icd_bus_s *new_bus = NULL;
	icd_bus_s *bus;

	RETV_IF(NULL == sender, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == handle, IOTCON_ERROR_INVALID_PARAMETER);

	cur_bus = icd_bus_list;

	while (cur_bus) {
		bus = cur_bus->data;
		if (NULL == bus) {
			ERR("bus is NULL");
			return IOTCON_ERROR_NO_DATA;
		}

		if (IC_STR_EQUAL == g_strcmp0(bus->sender, sender)) {
			DBG("sender(%s) already exist", sender);
			sender_exist = true;
			break;
		}

		cur_bus = cur_bus->next;
	}

	if (false == sender_exist) {
		DBG("sender(%s) not exist. make new one.", sender);

		new_bus = calloc(1, sizeof(icd_bus_s));
		if (NULL == new_bus) {
			ERR("calloc(bus) Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		sender_dup = ic_utils_strdup(sender);
		if (NULL == sender_dup) {
			ERR("ic_utils_strdup() Fail");
			free(new_bus);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		new_bus->sender = sender_dup;
		DBG("new bus(%s, %d) added", sender, signal_number);
		bus = new_bus;
	}

	cur_hd = bus->hdlist;
	while (cur_hd) {
		rsrc_handle = cur_hd->data;

		if (rsrc_handle->handle == GINT_TO_POINTER(handle)) {
			ERR("resource handle(%u, %u) already exist", rsrc_handle->handle, rsrc_handle->number);
			return IOTCON_ERROR_ALREADY;
		}

		cur_hd = cur_hd->next;
	}

	rsrc_handle = calloc(1, sizeof(icd_resource_handle_s));
	if (NULL == rsrc_handle) {
		ERR("calloc(handle) Fail(%d)", errno);
		if (false == sender_exist) {
			free(new_bus->sender);
			free(new_bus);
		}
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	rsrc_handle->handle = handle;
	rsrc_handle->number = signal_number;

	DBG("handle(%u) added in the bus(%s, %u)", handle, sender, signal_number);

	bus->hdlist = g_list_append(bus->hdlist, rsrc_handle);

	if (false == sender_exist)
		icd_bus_list = g_list_append(icd_bus_list, bus);

	return IOTCON_ERROR_NONE;
}

static gboolean _dbus_handle_register_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *uri_path,
		const gchar* const *resource_types,
		gint ifaces,
		guchar properties,
		guint signal_number)
{
	FN_CALL;
	int ret;
	const gchar *sender;
	void *handle = NULL;

	handle = icd_ioty_register_resource(uri_path, resource_types, ifaces, properties);
	if (handle) {
		sender = g_dbus_method_invocation_get_sender(invocation);
		ret = _icd_dbus_resource_list_append_handle(sender, handle, signal_number);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_dbus_resource_list_append_handle() Fail(%d)", ret);

			ret = icd_ioty_unregister_resource(handle);
			if (IOTCON_ERROR_NONE != ret)
				ERR("icd_ioty_unregister_resource(%u) Fail(%d)", handle, ret);
		}
	}

	ic_dbus_complete_register_resource(object, invocation, GPOINTER_TO_INT(handle));

	return TRUE;
}


static gboolean _dbus_handle_unregister_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint resource)
{
	int ret;

	ret = icd_ioty_unregister_resource(GINT_TO_POINTER(resource));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unregister_resource(%u) Fail(%d)", resource, ret);
	else
		DBG("handle(%u) deregistered", resource);

	_icd_dbus_resource_handle_free(resource);

	ic_dbus_complete_unregister_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_interface(icDbus *object,
		GDBusMethodInvocation *invocation, gint resource, gint iface)
{
	int ret;

	ret = icd_ioty_bind_interface(GINT_TO_POINTER(resource), iface);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_interface() Fail(%d)", ret);

	ic_dbus_complete_bind_interface(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_type(icDbus *object,
		GDBusMethodInvocation *invocation, gint resource, const gchar *type)
{
	int ret;

	ret = icd_ioty_bind_type(GINT_TO_POINTER(resource), type);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_type() Fail(%d)", ret);

	ic_dbus_complete_bind_type(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_bind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint parent, gint child)
{
	int ret;

	ret = icd_ioty_bind_resource(GINT_TO_POINTER(parent), GINT_TO_POINTER(child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_bind_resource() Fail(%d)", ret);

	ic_dbus_complete_bind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_unbind_resource(icDbus *object,
		GDBusMethodInvocation *invocation, gint parent, gint child)
{
	int ret;

	ret = icd_ioty_unbind_resource(GINT_TO_POINTER(parent), GINT_TO_POINTER(child));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unbind_resource() Fail(%d)", ret);

	ic_dbus_complete_unbind_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_find_resource(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		const gchar *type,
		guint signal_number)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_find_resource(host_address, type, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_find_resource() Fail(%d)", ret);

	ic_dbus_complete_find_resource(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_get(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *client, GVariant *query, guint signal_number)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_get(client, query, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get() Fail(%d)", ret);

	ic_dbus_complete_get(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_put(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *client,
		const gchar *repr,
		GVariant *query,
		guint signal_number)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_put(client, repr, query, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_put() Fail(%d)", ret);

	ic_dbus_complete_put(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_post(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *client,
		const gchar *repr,
		GVariant *query,
		guint signal_number)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_post(client, repr, query, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_post() Fail(%d)", ret);

	ic_dbus_complete_post(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_delete(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *client,
		guint signal_number)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_delete(client, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_delete() Fail(%d)", ret);

	ic_dbus_complete_delete(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_observer_start(icDbus *object,
		GDBusMethodInvocation *invocation,
		GVariant *client,
		gint observe_type,
		GVariant *query,
		guint signal_number)
{
	int ret;
	int observe_h;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_observer_start(client, observe_type, query, signal_number, sender,
			&observe_h);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_observer_start() Fail(%d)", ret);

	ic_dbus_complete_observer_start(object, invocation, observe_h, ret);

	return TRUE;
}


static gboolean _dbus_handle_observer_stop(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint observe_h)
{
	int ret;

	ret = icd_ioty_observer_stop(GINT_TO_POINTER(observe_h));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_observer_stop() Fail(%d)", ret);

	ic_dbus_complete_observer_stop(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_notify_list_of_observers(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint resource,
		GVariant *notify_msg,
		GVariant *observers)
{
	int ret;

	ret = icd_ioty_notify_list_of_observers(resource, notify_msg, observers);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_notify_list_of_observers() Fail(%d)", ret);

	ic_dbus_complete_notify_list_of_observers(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_notify_all(icDbus *object, GDBusMethodInvocation *invocation,
		gint resource)
{
	int ret;

	ret = icd_ioty_notify_all(resource);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_notify_all() Fail(%d)", ret);

	ic_dbus_complete_notify_all(object, invocation, ret);

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

#ifdef DEVICE_INFO_IMPL /* not implemented in iotivity 0.9.1 */
static gboolean _dbus_handle_register_device_info(icDbus *object,
		GDBusMethodInvocation *invocation, GVariant *device_info)
{
	int ret;

	ret = icd_ioty_register_device_info(device_info);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_register_device_info() Fail(%d)", ret);

	ic_dbus_complete_register_device_info(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_get_device_info(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		guint signal_number)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_get_device_info(host_address, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get_device_info() Fail(%d)", ret);

	ic_dbus_complete_get_device_info(object, invocation, ret);

	return TRUE;
}
#endif

static gboolean _dbus_handle_register_platform_info(icDbus *object,
		GDBusMethodInvocation *invocation, GVariant *platform_info)
{
	int ret;

	ret = icd_ioty_register_platform_info(platform_info);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_register_platform_info() Fail(%d)", ret);

	ic_dbus_complete_register_platform_info(object, invocation, ret);

	return TRUE;
}


static gboolean _dbus_handle_get_platform_info(icDbus *object,
		GDBusMethodInvocation *invocation,
		const gchar *host_address,
		guint signal_number)
{
	int ret;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	ret = icd_ioty_get_platform_info(host_address, signal_number, sender);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_get_platform_info() Fail(%d)", ret);

	ic_dbus_complete_get_platform_info(object, invocation, ret);

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
		const gchar *type,
		guint signal_number)
{
	void *presence_h;
	const gchar *sender;

	sender = g_dbus_method_invocation_get_sender(invocation);
	presence_h = icd_ioty_subscribe_presence(host_address, type, signal_number, sender);
	if (NULL == presence_h)
		ERR("icd_ioty_subscribe_presence() Fail");

	ic_dbus_complete_subscribe_presence(object, invocation,
			GPOINTER_TO_INT(presence_h));

	return TRUE;
}


static gboolean _dbus_handle_unsubscribe_presence(icDbus *object,
		GDBusMethodInvocation *invocation,
		gint presence_h)
{
	int ret;

	ret = icd_ioty_unsubscribe_presence(GINT_TO_POINTER(presence_h));
	if (IOTCON_ERROR_NONE != ret)
		ERR("icd_ioty_unsubscribe_presence() Fail(%d)", ret);

	ic_dbus_complete_unsubscribe_presence(object, invocation, ret);

	return TRUE;
}


static void _dbus_on_bus_acquired(GDBusConnection *conn, const gchar *name,
		gpointer user_data)
{
	gboolean ret;
	icDbus *dbus_object;
	GError *error = NULL;


	icd_dbus_conn = conn;

	dbus_object = ic_dbus_skeleton_new();
	if (NULL == dbus_object) {
		ERR("ic_iotcon_skeletion_new() Fail");
		return;
	}

	g_signal_connect(dbus_object, "handle-register-resource",
			G_CALLBACK(_dbus_handle_register_resource), NULL);
	g_signal_connect(dbus_object, "handle-unregister-resource",
			G_CALLBACK(_dbus_handle_unregister_resource), NULL);
	g_signal_connect(dbus_object, "handle-bind-interface",
			G_CALLBACK(_dbus_handle_bind_interface), NULL);
	g_signal_connect(dbus_object, "handle-bind-type",
			G_CALLBACK(_dbus_handle_bind_type), NULL);
	g_signal_connect(dbus_object, "handle-bind-resource",
			G_CALLBACK(_dbus_handle_bind_resource), NULL);
	g_signal_connect(dbus_object, "handle-unbind-resource",
			G_CALLBACK(_dbus_handle_unbind_resource), NULL);
	g_signal_connect(dbus_object, "handle-find-resource",
			G_CALLBACK(_dbus_handle_find_resource), NULL);
	g_signal_connect(dbus_object, "handle-get",
			G_CALLBACK(_dbus_handle_get), NULL);
	g_signal_connect(dbus_object, "handle-put",
			G_CALLBACK(_dbus_handle_put), NULL);
	g_signal_connect(dbus_object, "handle-post",
			G_CALLBACK(_dbus_handle_post), NULL);
	g_signal_connect(dbus_object, "handle-delete",
			G_CALLBACK(_dbus_handle_delete), NULL);
	g_signal_connect(dbus_object, "handle-observer-start",
			G_CALLBACK(_dbus_handle_observer_start), NULL);
	g_signal_connect(dbus_object, "handle-observer-stop",
			G_CALLBACK(_dbus_handle_observer_stop), NULL);
	g_signal_connect(dbus_object, "handle-notify-list-of-observers",
			G_CALLBACK(_dbus_handle_notify_list_of_observers), NULL);
	g_signal_connect(dbus_object, "handle-notify-all",
			G_CALLBACK(_dbus_handle_notify_all), NULL);
	g_signal_connect(dbus_object, "handle-send-response",
			G_CALLBACK(_dbus_handle_send_response), NULL);
#ifdef DEVICE_INFO_IMPL /* not implemented in iotivity 0.9.1 */
	g_signal_connect(dbus_object, "handle-register-device-info",
			G_CALLBACK(_dbus_handle_register_device_info), NULL);
	g_signal_connect(dbus_object, "handle-get-device-info",
			G_CALLBACK(_dbus_handle_get_device_info), NULL);
#endif
	g_signal_connect(dbus_object, "handle-register-platform-info",
			G_CALLBACK(_dbus_handle_register_platform_info), NULL);
	g_signal_connect(dbus_object, "handle-get-platform-info",
			G_CALLBACK(_dbus_handle_get_platform_info), NULL);
	g_signal_connect(dbus_object, "handle-start-presence",
			G_CALLBACK(_dbus_handle_start_presence), NULL);
	g_signal_connect(dbus_object, "handle-stop-presence",
			G_CALLBACK(_dbus_handle_stop_presence), NULL);
	g_signal_connect(dbus_object, "handle-subscribe-presence",
			G_CALLBACK(_dbus_handle_subscribe_presence), NULL);
	g_signal_connect(dbus_object, "handle-unsubscribe-presence",
			G_CALLBACK(_dbus_handle_unsubscribe_presence), NULL);

	ret = g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(dbus_object), conn,
			IOTCON_DBUS_OBJPATH, &error);
	if (FALSE == ret) {
		ERR("g_dbus_interface_skeleton_export() Fail(%s)", error->message);
		g_error_free(error);
	}

	ret = _icd_dbus_subscribe_name_owner_changed();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_dbus_subscribe_name_owner_changed() Fail(%d)", ret);
		return;
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

