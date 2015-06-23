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
#include <gio/gio.h>

#include "iotcon.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-dbus.h"
#include "icl.h"
#include "icl-dbus-type.h"
#include "icl-dbus.h"
#include "icl-client.h"
#include "icl-repr.h"
#include "icl-request.h"

static GDBusConnection *icl_dbus_conn;
static int icl_dbus_count;
static icDbus *icl_dbus_object;

typedef struct {
	void *cb;
	void *user_data;
	unsigned int id;
} icl_cb_container_s;


static inline unsigned int _icl_dbus_generate_signal_number()
{
	static unsigned int i = 0;

	return i++;
}


static inline int _icl_dbus_get()
{
	if (NULL == icl_dbus_conn) {
		icl_dbus_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
		if (NULL == icl_dbus_conn) {
			ERR("g_bus_get_sync() Fail");
			return IOTCON_ERROR_DBUS;
		}
	}

	icl_dbus_count++;

	return IOTCON_ERROR_NONE;
}


static inline void _icl_dbus_unref()
{
	icl_dbus_count--;

	if (0 == icl_dbus_count) {
		DBG("All connection is closed");
		g_object_unref(icl_dbus_conn);
		icl_dbus_conn = NULL;
	}
}


static unsigned int _icl_dbus_subscribe_signal(char *sig_name, void *cb, void *user_data,
		GDBusSignalCallback sig_handler)
{
	unsigned int id;
	icl_cb_container_s *cb_container;

	cb_container = calloc(1, sizeof(icl_cb_container_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return 0;
	}
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	id = g_dbus_connection_signal_subscribe(icl_dbus_conn,
			NULL,
			IOTCON_DBUS_INTERFACE,
			sig_name,
			IOTCON_DBUS_OBJPATH,
			NULL,
			G_DBUS_SIGNAL_FLAGS_NONE,
			sig_handler,
			cb_container,
			free);
	if (0 == id) {
		ERR("g_dbus_connection_signal_subscribe() Fail");
		free(cb_container);
		return id;
	}

	cb_container->id = id;

	return id;
}


static void _icl_dbus_request_handler(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;

	int index = 0;
	GVariantIter *options;
	unsigned short option_id;
	char *option_data;
	GVariantIter *query;
	char *key = NULL;
	char *value = NULL;
	GVariantIter *repr;
	char *repr_json;
	char *repr_uri;
	int request_handle;
	int resource_handle;
	struct icl_resource_request request = {0};
	icl_cb_container_s *cb_container = user_data;
	iotcon_request_handler_cb cb = cb_container->cb;

	g_variant_get(parameters, "(i&sa(qs)a(ss)iiasii)",
			&request.types,
			&request.uri,
			&options,
			&query,
			&request.observation_info.action,
			&request.observation_info.observer_id,
			&repr,
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

	for (index = 0; g_variant_iter_loop(repr, "&s", &repr_json); index++) {
		iotcon_repr_h cur_repr = icl_repr_parse_json(repr_json);
		if (NULL == cur_repr) {
			ERR("icl_repr_parse_json() Fail");
			iotcon_options_free(request.header_options);
			iotcon_query_free(request.query);
			if (request.repr)
				iotcon_repr_free(request.repr);
			g_variant_iter_free(repr);
			return;
		}
		repr_uri = icl_repr_json_get_uri(repr_json);
		iotcon_repr_set_uri(cur_repr, repr_uri);
		free(repr_uri);

		if (0 == index)
			request.repr = cur_repr;
		else
			request.repr->children = g_list_append(request.repr->children, cur_repr);
	}
	g_variant_iter_free(repr);

	if (cb)
		cb(&request, cb_container->user_data);

	/* To avoid unnecessary ERR log (repr could be NULL) */
	if (request.repr)
		iotcon_repr_free(request.repr);
	if (request.query)
		iotcon_query_free(request.query);
	if (request.header_options)
		iotcon_options_free(request.header_options);
}


icl_handle_container_s* icl_dbus_register_resource(const char *uri,
		iotcon_resource_types_h types,
		int ifaces,
		uint8_t properties,
		iotcon_request_handler_cb cb,
		void *user_data)
{
	int signal_number;
	unsigned int subscription_id;
	int resource_handle;
	GError *error = NULL;
	const gchar **res_types;
	char sig_name[IC_DBUS_SIGNAL_LENGTH];
	icl_handle_container_s *handle_container;

	RETV_IF(NULL == icl_dbus_object, NULL);

	signal_number = _icl_dbus_generate_signal_number();

	handle_container = calloc(1, sizeof(icl_handle_container_s));
	if (NULL == handle_container) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	res_types = icl_dbus_resource_types_to_array(types);
	if (NULL == res_types) {
		ERR("icl_dbus_resource_types_to_array() Fail");
		free(handle_container);
		return NULL;
	}

	ic_dbus_call_register_resource_sync(icl_dbus_object, uri, res_types,
			ifaces, properties, signal_number, &resource_handle, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_register_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		free(res_types);
		free(handle_container);
		return NULL;
	}
	free(res_types);

	snprintf(sig_name, sizeof(sig_name), "%s_%u", IC_DBUS_SIGNAL_REQUEST_HANDLER,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(sig_name, cb, user_data,
			_icl_dbus_request_handler);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		free(handle_container);
		return NULL;
	}

	handle_container->handle = resource_handle;
	handle_container->id = subscription_id;

	return handle_container;
}


int icl_dbus_unregister_resource(icl_handle_container_s *resource)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_unregister_resource_sync(icl_dbus_object, resource->handle, &ret,
			NULL, &error);
	if (error) {
		ERR("ic_dbus_call_unregister_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	g_dbus_connection_signal_unsubscribe(icl_dbus_conn, resource->id);
	free(resource);

	return ret;
}


int icl_dbus_bind_interface(icl_handle_container_s *resource, int iface)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_bind_interface_sync(icl_dbus_object, resource->handle, iface, &ret,
			NULL, &error);
	if (error) {
		ERR("ic_dbus_call_bind_interface_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_bind_type(icl_handle_container_s *resource, const char *type)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_bind_type_sync(icl_dbus_object, resource->handle, type, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_bind_type_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_bind_resource(icl_handle_container_s *parent, icl_handle_container_s *child)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_bind_resource_sync(icl_dbus_object, parent->handle, child->handle,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_bind_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_unbind_resource(icl_handle_container_s *parent,
		icl_handle_container_s *child)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_unbind_resource_sync(icl_dbus_object, parent->handle, child->handle,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_unbind_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_notify_list_of_observers(icl_handle_container_s *resource,
		struct icl_notify_msg *msg, iotcon_observers_h observers)
{
	int ret;
	GError *error = NULL;
	GVariant *noti_msg;
	GVariant *obs;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	noti_msg = icl_dbus_notimsg_to_gvariant(msg);
	if (NULL == noti_msg) {
		ERR("icl_dbus_notimsg_to_gvariant() Fail");
		return IOTCON_ERROR_REPRESENTATION;
	}
	obs = icl_dbus_observers_to_gvariant(observers);

	ic_dbus_call_notify_list_of_observers_sync(icl_dbus_object, resource->handle,
			noti_msg, obs, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_notify_list_of_observers_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(obs);
		g_variant_unref(noti_msg);
		return IOTCON_ERROR_DBUS;
	}

	return IOTCON_ERROR_NONE;
}


int icl_dbus_notify_all(icl_handle_container_s *resource)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_notify_all_sync(icl_dbus_object, resource->handle, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_notify_all_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return IOTCON_ERROR_NONE;
}


int icl_dbus_send_response(struct icl_resource_response *response)
{
	int ret;
	GError *error = NULL;
	GVariant *arg_response;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	arg_response = icl_dbus_response_to_gvariant(response);
	ic_dbus_call_send_response_sync(icl_dbus_object, arg_response,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_send_response_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_response);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


static void _icl_dbus_found_resource(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	char *type;
	GVariantIter *types;

	icl_cb_container_s *cb_container = user_data;
	iotcon_found_resource_cb cb = cb_container->cb;

	struct icl_remote_resource resource = {0};

	g_variant_get(parameters, "(&s&siasi)",
			&resource.uri,
			&resource.host,
			&resource.is_observable,
			&types,
			&resource.ifaces);

	if (g_variant_iter_n_children(types)) {
		resource.types = iotcon_resource_types_new();
		while (g_variant_iter_loop(types, "&s", &type))
			iotcon_resource_types_insert(resource.types, type);
	}
	g_variant_iter_free(types);

	if (cb)
		cb(&resource, cb_container->user_data);

	if (resource.types)
		iotcon_resource_types_free(resource.types);
}


int icl_dbus_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb cb, void *user_data)
{
	int ret;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	signal_number = _icl_dbus_generate_signal_number();

	ic_dbus_call_find_resource_sync(icl_dbus_object, host_address,
			ic_utils_dbus_encode_str(resource_type), signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_find_resource_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_FOUND_RESOURCE,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data,
			_icl_dbus_found_resource);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


static void _icl_dbus_on_cru(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	int index;
	GVariantIter *options;
	unsigned short option_id;
	char *option_data;
	iotcon_options_h header_options = NULL;
	iotcon_repr_h repr = NULL;
	GVariantIter *reprIter;
	char *repr_json;
	char *repr_uri;
	int res;

	icl_cb_container_s *cb_container = user_data;
	iotcon_on_cru_cb cb = cb_container->cb;

	g_dbus_connection_signal_unsubscribe(icl_dbus_conn, cb_container->id);

	g_variant_get(parameters, "(a(qs)asi)", &options, &reprIter, &res);

	if (g_variant_iter_n_children(options)) {
		header_options = iotcon_options_new();
		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_insert(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	for (index = 0; g_variant_iter_loop(reprIter, "&s", &repr_json); index++) {
		iotcon_repr_h cur_repr = icl_repr_parse_json(repr_json);
		if (NULL == cur_repr) {
			ERR("icl_repr_parse_json() Fail");
			iotcon_options_free(header_options);
			if (repr)
				iotcon_repr_free(repr);
			g_variant_iter_free(reprIter);
			return;
		}
		repr_uri = icl_repr_json_get_uri(repr_json);
		iotcon_repr_set_uri(cur_repr, repr_uri);
		free(repr_uri);

		if (0 == index)
			repr = cur_repr;
		else
			repr->children = g_list_append(repr->children, cur_repr);
	}
	g_variant_iter_free(reprIter);

	if (cb)
		cb(header_options, repr, res, cb_container->user_data);

	if (repr)
		iotcon_repr_free(repr);
	if (header_options)
		iotcon_options_free(header_options);
}


int icl_dbus_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	signal_number = _icl_dbus_generate_signal_number();

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);
	ic_dbus_call_get_sync(icl_dbus_object, arg_client, arg_query, signal_number,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_get_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_query);
		g_variant_unref(arg_client);
		return IOTCON_ERROR_DBUS;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_GET,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data, _icl_dbus_on_cru);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_put(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	char *arg_repr;
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	signal_number = _icl_dbus_generate_signal_number();

	arg_repr = icl_repr_generate_json(repr, FALSE);
	if (NULL == arg_repr) {
		ERR("icl_repr_generate_json() Fail");
		return IOTCON_ERROR_REPRESENTATION;
	}

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_put_sync(icl_dbus_object, arg_client, arg_repr, arg_query,
			signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_put_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_query);
		g_variant_unref(arg_client);
		free(arg_repr);
		return IOTCON_ERROR_DBUS;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_PUT,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data,
			_icl_dbus_on_cru);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		free(arg_repr);
		return IOTCON_ERROR_DBUS;
	}

	free(arg_repr);

	return ret;
}


int icl_dbus_post(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	char *arg_repr;
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	signal_number = _icl_dbus_generate_signal_number();

	arg_repr = icl_repr_generate_json(repr, FALSE);
	if (NULL == arg_repr) {
		ERR("icl_repr_generate_json() Fail");
		return IOTCON_ERROR_REPRESENTATION;
	}

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_post_sync(icl_dbus_object, arg_client, arg_repr, arg_query,
			signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_post_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_client);
		g_variant_unref(arg_query);
		free(arg_repr);
		return IOTCON_ERROR_DBUS;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_POST,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data,
			_icl_dbus_on_cru);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		free(arg_repr);
		return IOTCON_ERROR_DBUS;
	}

	free(arg_repr);

	return ret;
}


static void _icl_dbus_on_delete(GDBusConnection *connection,
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
	iotcon_options_h header_options = NULL;
	int res;

	icl_cb_container_s *cb_container = user_data;
	iotcon_on_delete_cb cb = cb_container->cb;

	g_dbus_connection_signal_unsubscribe(icl_dbus_conn, cb_container->id);

	g_variant_get(parameters, "(a(qs)i)", &options, &res);

	if (g_variant_iter_n_children(options)) {
		header_options = iotcon_options_new();
		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_insert(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	if (cb)
		cb(header_options, res, cb_container->user_data);

	if (header_options)
		iotcon_options_free(header_options);
}


int icl_dbus_delete(iotcon_client_h resource, iotcon_on_delete_cb cb,
		void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	GVariant *arg_client;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	signal_number = _icl_dbus_generate_signal_number();

	arg_client = icl_dbus_client_to_gvariant(resource);

	ic_dbus_call_delete_sync(icl_dbus_object, arg_client, signal_number, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_delete_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_client);
		return IOTCON_ERROR_DBUS;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_DELETE,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data,
			_icl_dbus_on_delete);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


static void _icl_dbus_on_observe(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	int index;
	GVariantIter *options;
	unsigned short option_id;
	char *option_data;
	iotcon_options_h header_options = NULL;
	iotcon_repr_h repr = NULL;
	GVariantIter *reprIter;
	char *repr_json;
	char *repr_uri;
	int res;
	int seq_num;

	icl_cb_container_s *cb_container = user_data;
	iotcon_on_observe_cb cb = cb_container->cb;

	g_variant_get(parameters, "(a(qs)asii)", &options, &reprIter, &res, &seq_num);

	if (g_variant_iter_n_children(options)) {
		header_options = iotcon_options_new();
		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_insert(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	for (index = 0; g_variant_iter_loop(reprIter, "&s", &repr_json); index++) {
		iotcon_repr_h cur_repr = icl_repr_parse_json(repr_json);
		if (NULL == cur_repr) {
			ERR("icl_repr_parse_json() Fail");
			iotcon_options_free(header_options);
			if (repr)
				iotcon_repr_free(repr);
			g_variant_iter_free(reprIter);
			return;
		}
		repr_uri = icl_repr_json_get_uri(repr_json);
		iotcon_repr_set_uri(cur_repr, repr_uri);
		free(repr_uri);

		if (0 == index)
			repr = cur_repr;
		else
			repr->children = g_list_append(repr->children, cur_repr);
	}
	g_variant_iter_free(reprIter);

	if (cb)
		cb(header_options, repr, res, seq_num, cb_container->user_data);

	if (repr)
		iotcon_repr_free(repr);
	if (header_options)
		iotcon_options_free(header_options);
}


int icl_dbus_observer_start(iotcon_client_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_on_observe_cb cb,
		void *user_data)
{
	int ret;
	int observe_h;
	GError *error = NULL;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	signal_number = _icl_dbus_generate_signal_number();

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_observer_start_sync(icl_dbus_object, arg_client, observe_type,
			arg_query, signal_number, &observe_h, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_observer_start_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_query);
		g_variant_unref(arg_client);
		return IOTCON_ERROR_DBUS;
	}

	resource->observe_handle = observe_h;

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_OBSERVE,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data,
			_icl_dbus_on_observe);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_observer_stop(iotcon_client_h resource)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_observer_stop_sync(icl_dbus_object,
			resource->observe_handle, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_observer_stop_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}
	if (IOTCON_ERROR_NONE == ret)
		resource->observe_handle = 0;

	return ret;
}


int icl_dbus_register_device_info(iotcon_device_info_s info)
{
	int ret;
	GError *error = NULL;
	GVariant *arg_info;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	arg_info = icl_dbus_device_info_to_gvariant(&info);
	ic_dbus_call_register_device_info_sync(icl_dbus_object, arg_info, &ret,
			NULL, &error);
	if (error) {
		ERR("ic_dbus_call_register_device_info_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_info);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


static void _icl_dbus_received_device_info(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	icl_cb_container_s *cb_container = user_data;
	iotcon_device_info_cb cb = cb_container->cb;

	iotcon_device_info_s info = {0};

	g_variant_get(parameters, "(&s&s&s&s&s&s&s&s&s&s&s&s)",
			&info.name,
			&info.host_name,
			&info.uuid,
			&info.content_type,
			&info.version,
			&info.manuf_name,
			&info.manuf_url,
			&info.model_number,
			&info.date_of_manufacture,
			&info.platform_ver,
			&info.firmware_ver,
			&info.support_url);

	info.name = ic_utils_dbus_decode_str(info.name);
	info.host_name = ic_utils_dbus_decode_str(info.host_name);
	info.uuid = ic_utils_dbus_decode_str(info.uuid);
	info.content_type = ic_utils_dbus_decode_str(info.content_type);
	info.version = ic_utils_dbus_decode_str(info.version);
	info.manuf_name = ic_utils_dbus_decode_str(info.manuf_name);
	info.manuf_url = ic_utils_dbus_decode_str(info.manuf_url);
	info.model_number = ic_utils_dbus_decode_str(info.model_number);
	info.date_of_manufacture = ic_utils_dbus_decode_str(info.date_of_manufacture);
	info.platform_ver = ic_utils_dbus_decode_str(info.platform_ver);
	info.firmware_ver = ic_utils_dbus_decode_str(info.firmware_ver);
	info.support_url = ic_utils_dbus_decode_str(info.support_url);

	if (cb)
		cb(info, cb_container->user_data);
}


int icl_dbus_get_device_info(const char *host_address, iotcon_device_info_cb cb,
		void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	signal_number = _icl_dbus_generate_signal_number();

	ic_dbus_call_get_device_info_sync(icl_dbus_object, host_address,
			signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_get_device_info_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_DEVICE,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data,
			_icl_dbus_received_device_info);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_start_presence(unsigned int time_to_live)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_start_presence_sync(icl_dbus_object, time_to_live, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_start_presence_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


int icl_dbus_stop_presence()
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_stop_presence_sync(icl_dbus_object, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_stop_presence_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


static void _icl_dbus_presence_handler(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	int res;
	unsigned int nonce;
	char *host_address;
	icl_cb_container_s *cb_container = user_data;
	iotcon_presence_cb cb = cb_container->cb;

	g_variant_get(parameters, "(iu&s)", &res, &nonce, &host_address);

	if (cb)
		cb(res, nonce, host_address, cb_container->user_data);
}


icl_handle_container_s* icl_dbus_subscribe_presence(const char *host_address,
		const char *type, iotcon_presence_cb cb, void *user_data)
{
	int presence_h;
	GError *error = NULL;
	unsigned int subscription_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	icl_handle_container_s *handle_container;

	RETV_IF(NULL == icl_dbus_object, NULL);

	signal_number = _icl_dbus_generate_signal_number();

	handle_container = calloc(1, sizeof(icl_handle_container_s));
	if (NULL == handle_container) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	ic_dbus_call_subscribe_presence_sync(icl_dbus_object, host_address, type,
			signal_number, &presence_h, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_subscribe_presence_sync() Fail(%s)", error->message);
		g_error_free(error);
		free(handle_container);
		return NULL;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_PRESENCE,
			signal_number);

	subscription_id = _icl_dbus_subscribe_signal(signal_name, cb, user_data,
			_icl_dbus_presence_handler);
	if (0 == subscription_id) {
		ERR("_icl_dbus_subscribe_signal() Fail");
		free(handle_container);
		return NULL;
	}

	handle_container->handle = presence_h;
	handle_container->id = subscription_id;

	return handle_container;
}


int icl_dbus_unsubscribe_presence(icl_handle_container_s *presence_h)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_object, IOTCON_ERROR_DBUS);

	ic_dbus_call_unsubscribe_presence_sync(icl_dbus_object, presence_h->handle,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_unsubscribe_presence_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	g_dbus_connection_signal_unsubscribe(icl_dbus_conn, presence_h->id);
	free(presence_h);

	return ret;
}


unsigned int icl_dbus_start()
{
	int ret;
	GError *error = NULL;

	ret = _icl_dbus_get();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_dbus_get() Fail(%d)", ret);
		return ret;
	}

	icl_dbus_object = ic_dbus_proxy_new_sync(icl_dbus_conn,
			G_DBUS_PROXY_FLAGS_NONE,
			IOTCON_DBUS_INTERFACE,
			IOTCON_DBUS_OBJPATH,
			NULL,
			&error);
	if (NULL == icl_dbus_object) {
		ERR("ic_iotcon_proxy_new_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	return ret;
}


void icl_dbus_stop()
{
	g_object_unref(icl_dbus_object);
	icl_dbus_object = NULL;
	_icl_dbus_unref();
}

