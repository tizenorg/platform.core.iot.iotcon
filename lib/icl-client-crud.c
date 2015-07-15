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
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-options.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-repr.h"
#include "icl-client.h"

typedef struct {
	iotcon_on_cru_cb cb;
	void *user_data;
	iotcon_client_h resource;
	unsigned int id;
} icl_on_cru_s;

typedef struct {
	iotcon_on_delete_cb cb;
	void *user_data;
	iotcon_client_h resource;
	unsigned int id;
} icl_on_delete_s;

typedef struct {
	iotcon_on_observe_cb cb;
	void *user_data;
	iotcon_client_h resource;
} icl_on_observe_s;


static void _icl_on_cru_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	int res;
	GVariantIter *options;
	unsigned short option_id;
	char *option_data;
	iotcon_options_h header_options = NULL;
	iotcon_repr_h repr = NULL;
	char *repr_json = NULL;

	icl_on_cru_s *cb_container = user_data;
	iotcon_on_cru_cb cb = cb_container->cb;

	icl_dbus_unsubscribe_signal(cb_container->id);

	g_variant_get(parameters, "(a(qs)si)", &options, &repr_json, &res);

	if (IOTCON_ERROR_NONE == res && g_variant_iter_n_children(options)) {
		header_options = iotcon_options_new();
		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_insert(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	if (IC_STR_EQUAL == strcmp(IC_STR_NULL, repr_json)) {
		repr = iotcon_repr_new();
	} else {
		repr = icl_repr_create_repr(repr_json);
		if (NULL == repr) {
			ERR("icl_repr_create_repr() Fail");
			if (header_options)
				iotcon_options_free(header_options);
			return;
		}
	}

	res = icl_dbus_convert_daemon_error(res);

	if (cb)
		cb(cb_container->resource, repr, header_options, res, cb_container->user_data);

	if (repr)
		iotcon_repr_free(repr);
	if (header_options)
		iotcon_options_free(header_options);

}


static void _icl_cru_conn_cleanup(icl_on_cru_s *cb_container)
{
	iotcon_client_free(cb_container->resource);
	free(cb_container);
}


API int iotcon_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	icl_on_cru_s *cb_container;
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	signal_number = icl_dbus_generate_signal_number();

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_get_sync(icl_dbus_get_object(), arg_client, arg_query, signal_number,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_get_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_query);
		g_variant_unref(arg_client);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_GET,
			signal_number);

	cb_container = calloc(1, sizeof(icl_on_cru_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = iotcon_client_ref(resource);
	if (NULL == cb_container->resource) {
		ERR("iotcon_client_ref() Fail");
		free(cb_container);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, _icl_cru_conn_cleanup,
			_icl_on_cru_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		_icl_cru_conn_cleanup(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}


API int iotcon_put(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	char *arg_repr;
	icl_on_cru_s *cb_container;
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	signal_number = icl_dbus_generate_signal_number();

	arg_repr = icl_repr_generate_json(repr, FALSE);
	if (NULL == arg_repr) {
		ERR("icl_repr_generate_json() Fail");
		return IOTCON_ERROR_REPRESENTATION;
	}

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_put_sync(icl_dbus_get_object(), arg_client, arg_repr, arg_query,
			signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_put_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_query);
		g_variant_unref(arg_client);
		free(arg_repr);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		free(arg_repr);
		return icl_dbus_convert_daemon_error(ret);
	}
	free(arg_repr);

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_PUT,
			signal_number);

	cb_container = calloc(1, sizeof(icl_on_cru_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = iotcon_client_ref(resource);
	if (NULL == cb_container->resource) {
		ERR("iotcon_client_ref() Fail");
		free(cb_container);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, _icl_cru_conn_cleanup,
			_icl_on_cru_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		_icl_cru_conn_cleanup(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}


API int iotcon_post(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	char *arg_repr;
	icl_on_cru_s *cb_container;
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	signal_number = icl_dbus_generate_signal_number();

	arg_repr = icl_repr_generate_json(repr, FALSE);
	if (NULL == arg_repr) {
		ERR("icl_repr_generate_json() Fail");
		return IOTCON_ERROR_REPRESENTATION;
	}

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_post_sync(icl_dbus_get_object(), arg_client, arg_repr, arg_query,
			signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_post_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_client);
		g_variant_unref(arg_query);
		free(arg_repr);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		free(arg_repr);
		return icl_dbus_convert_daemon_error(ret);
	}
	free(arg_repr);

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_POST,
			signal_number);

	cb_container = calloc(1, sizeof(icl_on_cru_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = iotcon_client_ref(resource);
	if (NULL == cb_container->resource) {
		ERR("iotcon_client_ref() Fail");
		free(cb_container);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, _icl_cru_conn_cleanup,
			_icl_on_cru_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		_icl_cru_conn_cleanup(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}


static void _icl_on_delete_cb(GDBusConnection *connection,
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

	icl_on_delete_s *cb_container = user_data;
	iotcon_on_delete_cb cb = cb_container->cb;

	g_variant_get(parameters, "(a(qs)i)", &options, &res);

	if (g_variant_iter_n_children(options)) {
		header_options = iotcon_options_new();
		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_insert(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	if (cb)
		cb(cb_container->resource, header_options, res, cb_container->user_data);

	if (header_options)
		iotcon_options_free(header_options);

	icl_dbus_unsubscribe_signal(cb_container->id);
}


static void _icl_delete_conn_cleanup(icl_on_delete_s *cb_container)
{
	iotcon_client_free(cb_container->resource);
	free(cb_container);
}


API int iotcon_delete(iotcon_client_h resource, iotcon_on_delete_cb cb, void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	icl_on_delete_s *cb_container;
	GVariant *arg_client;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	signal_number = icl_dbus_generate_signal_number();

	arg_client = icl_dbus_client_to_gvariant(resource);

	ic_dbus_call_delete_sync(icl_dbus_get_object(), arg_client, signal_number, &ret,
			NULL, &error);
	if (error) {
		ERR("ic_dbus_call_delete_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_client);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_DELETE,
			signal_number);

	cb_container = calloc(1, sizeof(icl_on_delete_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = iotcon_client_ref(resource);
	if (NULL == cb_container->resource) {
		ERR("iotcon_client_ref() Fail");
		free(cb_container);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, _icl_delete_conn_cleanup,
			_icl_on_delete_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		_icl_delete_conn_cleanup(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}


static void _icl_on_observe_cb(GDBusConnection *connection,
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
	GVariantIter *repr_iter;
	char *repr_json;
	char *repr_uri_path;
	int res;
	int seq_num;

	icl_on_observe_s *cb_container = user_data;
	iotcon_on_observe_cb cb = cb_container->cb;

	g_variant_get(parameters, "(a(qs)asii)", &options, &repr_iter, &res, &seq_num);

	if (g_variant_iter_n_children(options)) {
		header_options = iotcon_options_new();
		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_insert(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	for (index = 0; g_variant_iter_loop(repr_iter, "&s", &repr_json); index++) {
		iotcon_repr_h cur_repr = icl_repr_parse_json(repr_json);
		if (NULL == cur_repr) {
			ERR("icl_repr_parse_json() Fail");
			iotcon_options_free(header_options);
			if (repr)
				iotcon_repr_free(repr);
			g_variant_iter_free(repr_iter);
			return;
		}
		repr_uri_path = icl_repr_json_get_uri_path(repr_json);
		iotcon_repr_set_uri_path(cur_repr, repr_uri_path);
		free(repr_uri_path);

		if (0 == index)
			repr = cur_repr;
		else
			repr->children = g_list_append(repr->children, cur_repr);
	}
	g_variant_iter_free(repr_iter);

	if (cb)
		cb(cb_container->resource, repr, header_options, res, seq_num,
				cb_container->user_data);

	if (repr)
		iotcon_repr_free(repr);
	if (header_options)
		iotcon_options_free(header_options);
}


static void _icl_observe_conn_cleanup(icl_on_observe_s *cb_container)
{
	cb_container->resource->observe_handle = 0;
	cb_container->resource->observe_sub_id = 0;
	iotcon_client_free(cb_container->resource);
	free(cb_container);
}


API int iotcon_observer_start(iotcon_client_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_on_observe_cb cb,
		void *user_data)
{
	int ret;
	int observe_handle;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	icl_on_observe_s *cb_container;
	GVariant *arg_client;
	GVariant *arg_query;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	signal_number = icl_dbus_generate_signal_number();

	arg_client = icl_dbus_client_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_observer_start_sync(icl_dbus_get_object(), arg_client, observe_type,
			arg_query, signal_number, &observe_handle, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_observer_start_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_query);
		g_variant_unref(arg_client);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_OBSERVE,
			signal_number);

	cb_container = calloc(1, sizeof(icl_on_observe_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = iotcon_client_ref(resource);
	if (NULL == cb_container->resource) {
		ERR("iotcon_client_ref() Fail");
		free(cb_container);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, _icl_observe_conn_cleanup,
			_icl_on_observe_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}
	resource->observe_sub_id = sub_id;
	resource->observe_handle = observe_handle;

	return ret;
}


API int iotcon_observer_stop(iotcon_client_h resource)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (0 == resource->observe_handle) {
		ERR("It doesn't have a observe_handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ic_dbus_call_observer_stop_sync(icl_dbus_get_object(),
			resource->observe_handle, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_observer_stop_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	icl_dbus_unsubscribe_signal(resource->observe_sub_id);

	return ret;
}

