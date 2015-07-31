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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl-resource-types.h"
#include "icl-resource.h"
#include "icl-request.h"
#include "icl-repr.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl.h"

static void _icl_request_handler(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	GVariantIter *options;
	unsigned short option_id;
	char *option_data;
	GVariantIter *query;
	char *key = NULL;
	char *value = NULL;
	char *repr_json;
	int request_handle;
	int resource_handle;
	struct icl_resource_request request = {0};
	iotcon_resource_h resource = user_data;
	iotcon_request_handler_cb cb = resource->cb;

	g_variant_get(parameters, "(ia(qs)a(ss)ii&sii)",
			&request.types,
			&options,
			&query,
			&request.observation_info.action,
			&request.observation_info.observer_id,
			&repr_json,
			&request_handle,
			&resource_handle);

	if (g_variant_iter_n_children(options)) {
		request.header_options = iotcon_options_new();
		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_insert(request.header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	if (g_variant_iter_n_children(query)) {
		request.query = iotcon_query_new();
		while (g_variant_iter_loop(query, "(&s&s)", &key, &value))
			iotcon_query_insert(request.query, key, value);
	}
	g_variant_iter_free(query);

	request.request_handle = GINT_TO_POINTER(request_handle);
	request.resource_handle = GINT_TO_POINTER(resource_handle);

	if (ic_utils_dbus_decode_str(repr_json)) {
		request.repr = icl_repr_create_repr(repr_json);
		if (NULL == request.repr) {
			ERR("icl_repr_create_repr() Fail");
			if (request.query)
				iotcon_query_free(request.query);
			if (request.header_options)
				iotcon_options_free(request.header_options);
			return;
		}
	}

	/* TODO remove request.uri */
	request.uri_path = "temp_uri_path";

	if (cb)
		cb(resource, &request, resource->user_data);

	/* To avoid unnecessary ERR log (repr could be NULL) */
	if (request.repr)
		iotcon_repr_free(request.repr);
	if (request.query)
		iotcon_query_free(request.query);
	if (request.header_options)
		iotcon_options_free(request.header_options);
}


static void _icl_resource_conn_cleanup(iotcon_resource_h resource)
{
	resource->sub_id = 0;
	resource->handle = 0;
}


/* The length of uri_path should be less than or equal to 36. */
API iotcon_resource_h iotcon_register_resource(const char *uri_path,
		iotcon_resource_types_h res_types,
		int ifaces,
		uint8_t properties,
		iotcon_request_handler_cb cb,
		void *user_data)
{
	FN_CALL;
	int signal_number;
	unsigned int sub_id;
	GError *error = NULL;
	const gchar **types;
	char sig_name[IC_DBUS_SIGNAL_LENGTH];
	iotcon_resource_h resource;

	RETV_IF(NULL == icl_dbus_get_object(), NULL);
	RETV_IF(NULL == uri_path, NULL);
	RETVM_IF(IOTCON_URI_PATH_LENGTH_MAX < strlen(uri_path), NULL, "Invalid uri_path(%s)",
			uri_path);
	RETV_IF(NULL == res_types, NULL);
	RETV_IF(NULL == cb, NULL);

	resource = calloc(1, sizeof(struct icl_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	types = icl_dbus_resource_types_to_array(res_types);
	if (NULL == types) {
		ERR("icl_dbus_resource_types_to_array() Fail");
		free(resource);
		return NULL;
	}

	signal_number = icl_dbus_generate_signal_number();

	ic_dbus_call_register_resource_sync(icl_dbus_get_object(), uri_path, types, ifaces,
			properties, signal_number, &(resource->handle), NULL, &error);
	if (error) {
		ERR("ic_dbus_call_register_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		free(types);
		free(resource);
		return NULL;
	}
	free(types);

	if (0 == resource->handle) {
		ERR("iotcon-daemon Fail");
		free(resource);
		return NULL;
	}

	resource->cb = cb;
	resource->user_data = user_data;

	resource->types = icl_resource_types_ref(res_types);
	resource->uri_path = ic_utils_strdup(uri_path);
	resource->ifaces = ifaces;
	resource->is_observable = properties & IOTCON_OBSERVABLE;

	snprintf(sig_name, sizeof(sig_name), "%s_%u", IC_DBUS_SIGNAL_REQUEST_HANDLER,
			signal_number);

	sub_id = icl_dbus_subscribe_signal(sig_name, resource, _icl_resource_conn_cleanup,
			_icl_request_handler);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		free(resource);
		return NULL;
	}

	resource->sub_id = sub_id;

	return resource;
}


API int iotcon_unregister_resource(iotcon_resource_h resource)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == resource->sub_id) {
		WARN("Invalid Resource handle");
		iotcon_resource_types_free(resource->types);
		free(resource->uri_path);
		free(resource);
		return IOTCON_ERROR_NONE;
	}

	ic_dbus_call_unregister_resource_sync(icl_dbus_get_object(), resource->handle,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_unregister_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	icl_dbus_unsubscribe_signal(resource->sub_id);

	iotcon_resource_types_free(resource->types);
	free(resource->uri_path);
	free(resource);

	return IOTCON_ERROR_NONE;
}


API int iotcon_bind_interface(iotcon_resource_h resource, iotcon_interface_e iface)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (0 == resource->sub_id) {
		ERR("Invalid Resource handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ic_dbus_call_bind_interface_sync(icl_dbus_get_object(), resource->handle,
			iface, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_bind_interface_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return ret;
}


API int iotcon_bind_type(iotcon_resource_h resource, const char *resource_type)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_type, IOTCON_ERROR_INVALID_PARAMETER);
	if (IOTCON_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type)) {
		ERR("Invalid resource_type(%s)", resource_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if (0 == resource->sub_id) {
		ERR("Invalid Resource handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ic_dbus_call_bind_type_sync(icl_dbus_get_object(), resource->handle, resource_type,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_bind_type_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return ret;
}


API int iotcon_bind_request_handler(iotcon_resource_h resource,
		iotcon_request_handler_cb cb)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	WARN("Request handler is changed");
	resource->cb = cb;

	return IOTCON_ERROR_NONE;
}


API int iotcon_bind_resource(iotcon_resource_h parent, iotcon_resource_h child)
{
	FN_CALL;
	int ret;
	int i;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(parent == child, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == parent->sub_id) {
		ERR("Invalid Resource handle(parent)");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	if (0 == child->sub_id) {
		ERR("Invalid Resource handle(child)");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	for (i = 0; i < IOTCON_CONTAINED_RESOURCES_MAX; i++) {
		if (child == parent->children[i]) {
			ERR("Child resource was already bound to parent resource.");
			return IOTCON_ERROR_ALREADY;
		}
	}

	for (i = 0; i < IOTCON_CONTAINED_RESOURCES_MAX; i++) {
		if (NULL == parent->children[i]) {
			ic_dbus_call_bind_resource_sync(icl_dbus_get_object(), parent->handle,
					child->handle, &ret, NULL, &error);
			if (error) {
				ERR("ic_dbus_call_bind_resource_sync() Fail(%s)", error->message);
				g_error_free(error);
				return IOTCON_ERROR_DBUS;
			}

			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon-daemon Fail(%d)", ret);
				return icl_dbus_convert_daemon_error(ret);
			}

			parent->children[i] = child;

			return IOTCON_ERROR_NONE;
		}
	}

	ERR("There is no slot to bind a child resource");
	return IOTCON_ERROR_OUT_OF_MEMORY;
}


API int iotcon_unbind_resource(iotcon_resource_h parent, iotcon_resource_h child)
{
	int ret;
	int i;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == parent->sub_id) {
		ERR("Invalid Resource handle(parent)");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	if (0 == child->sub_id) {
		ERR("Invalid Resource handle(child)");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ic_dbus_call_unbind_resource_sync(icl_dbus_get_object(), parent->handle,
			child->handle, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_unbind_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	for (i = 0; i < IOTCON_CONTAINED_RESOURCES_MAX; i++) {
		if (child == parent->children[i])
			parent->children[i] = NULL;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_number_of_children(iotcon_resource_h resource, int *number)
{
	int i;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == number, IOTCON_ERROR_INVALID_PARAMETER);

	*number = 0;
	for (i = 0; i < IOTCON_CONTAINED_RESOURCES_MAX; i++) {
		if (resource->children[i])
			*number += 1;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_nth_child(iotcon_resource_h parent, int index,
		iotcon_resource_h *child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);
	if ((index < 0) || (IOTCON_CONTAINED_RESOURCES_MAX <= index)) {
		ERR("Invalid index(%d)", index);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*child = parent->children[index];

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_resource_get_uri_path(iotcon_resource_h resource, char **uri_path)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = resource->uri_path;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_resource_get_types(iotcon_resource_h resource,
		iotcon_resource_types_h *types)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = resource->types;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_interfaces(iotcon_resource_h resource, int *ifaces)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = resource->ifaces;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_is_observable(iotcon_resource_h resource, bool *observable)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == observable, IOTCON_ERROR_INVALID_PARAMETER);

	*observable = resource->is_observable;

	return IOTCON_ERROR_NONE;
}


API iotcon_notimsg_h iotcon_notimsg_new(iotcon_repr_h repr, iotcon_interface_e iface)
{
	iotcon_notimsg_h msg;

	RETV_IF(NULL == repr, NULL);

	msg = calloc(1, sizeof(struct icl_notify_msg));
	if (NULL == msg) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	msg->repr = repr;
	icl_repr_inc_ref_count(msg->repr);
	msg->iface = iface;
	msg->error_code = 200;

	return msg;
}


API void iotcon_notimsg_free(iotcon_notimsg_h msg)
{
	RET_IF(NULL == msg);

	iotcon_repr_free(msg->repr);
	free(msg);
}


API int iotcon_notify_list_of_observers(iotcon_resource_h resource, iotcon_notimsg_h msg,
		iotcon_observers_h observers)
{
	int ret;
	GError *error = NULL;
	GVariant *noti_msg;
	GVariant *obs;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == observers, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == resource->sub_id) {
		ERR("Invalid Resource handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	noti_msg = icl_dbus_notimsg_to_gvariant(msg);
	if (NULL == noti_msg) {
		ERR("icl_dbus_notimsg_to_gvariant() Fail");
		return IOTCON_ERROR_REPRESENTATION;
	}
	obs = icl_dbus_observers_to_gvariant(observers);

	ic_dbus_call_notify_list_of_observers_sync(icl_dbus_get_object(), resource->handle,
			noti_msg, obs, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_notify_list_of_observers_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(obs);
		g_variant_unref(noti_msg);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_notify_all(iotcon_resource_h resource)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (0 == resource->sub_id) {
		ERR("Invalid Resource handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ic_dbus_call_notify_all_sync(icl_dbus_get_object(), resource->handle, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_notify_all_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}
