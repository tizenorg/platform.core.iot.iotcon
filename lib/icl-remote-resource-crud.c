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
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include <gio/gio.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "ic-dbus.h"
#include "icl.h"
#include "icl-options.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-repr.h"
#include "icl-remote-resource.h"
#include "icl-payload.h"

typedef struct {
	iotcon_remote_resource_cru_cb cb;
	void *user_data;
	iotcon_remote_resource_h resource;
} icl_on_cru_s;

typedef struct {
	iotcon_remote_resource_delete_cb cb;
	void *user_data;
	iotcon_remote_resource_h resource;
} icl_on_delete_s;

typedef struct {
	iotcon_remote_resource_observe_cb cb;
	void *user_data;
	iotcon_remote_resource_h resource;
} icl_on_observe_s;

static GList *icl_cru_cb_list = NULL;
static GList *icl_delete_cb_list = NULL;

void icl_remote_resource_crud_stop(iotcon_remote_resource_h resource)
{
	GList *c;
	for (c = icl_cru_cb_list; c; c = c->next) {
		icl_on_cru_s *cb_container = c->data;
		if (NULL == cb_container) {
			ERR("cb_container is NULL");
			continue;
		}
		if (cb_container->resource == resource)
			cb_container->cb = NULL;
	}

	for (c = icl_delete_cb_list; c; c = c->next) {
		icl_on_delete_s *cb_container = c->data;
		if (NULL == cb_container) {
			ERR("cb_container is NULL");
			continue;
		}
		if (cb_container->resource == resource)
			cb_container->cb = NULL;
	}
}


static void _icl_on_cru_cb(GVariant *result, icl_on_cru_s *cb_container)
{
	int res, ret;
	iotcon_representation_h repr;
	GVariantIter *options;
	unsigned short option_id;
	char *option_data;
	GVariant *repr_gvar;
	iotcon_options_h header_options = NULL;
	iotcon_remote_resource_cru_cb cb = cb_container->cb;

	g_variant_get(result, "(a(qs)vi)", &options, &repr_gvar, &res);

	icl_cru_cb_list = g_list_remove(icl_cru_cb_list, cb_container);

	if (IOTCON_ERROR_NONE == res && g_variant_iter_n_children(options)) {
		ret = iotcon_options_create(&header_options);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_options_create() Fail(%d)", ret);
			free(cb_container);
			return;
		}

		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_add(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	repr = icl_representation_from_gvariant(repr_gvar);
	if (NULL == repr) {
		ERR("icl_representation_from_gvariant() Fail");
		if (header_options)
			iotcon_options_destroy(header_options);
		free(cb_container);
		return;
	}

	res = icl_dbus_convert_daemon_error(res);

	if (cb)
		cb(cb_container->resource, repr, header_options, res,
				cb_container->user_data);

	if (repr)
		iotcon_representation_destroy(repr);
	if (header_options)
		iotcon_options_destroy(header_options);

	free(cb_container);
}


static void _icl_on_get_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	GVariant *result;
	GError *error = NULL;
	icl_on_cru_s *cb_container = user_data;

	ic_dbus_call_get_finish(IC_DBUS(object), &result, g_async_res, &error);
	if (error) {
		ERR("ic_dbus_call_get_finish() Fail(%s)", error->message);
		if (cb_container->cb) {
			int ret = icl_dbus_convert_dbus_error(error->code);
			cb_container->cb(cb_container->resource, NULL, NULL, ret,
					cb_container->user_data);
		}
		g_error_free(error);
		free(cb_container);
		return;
	}

	_icl_on_cru_cb(result, cb_container);
}


static void _icl_on_put_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	GVariant *result;
	GError *error = NULL;
	icl_on_cru_s *cb_container = user_data;

	ic_dbus_call_put_finish(IC_DBUS(object), &result, g_async_res, &error);
	if (error) {
		ERR("ic_dbus_call_put_finish() Fail(%s)", error->message);
		if (cb_container->cb) {
			int ret = icl_dbus_convert_dbus_error(error->code);
			cb_container->cb(cb_container->resource, NULL, NULL, ret,
					cb_container->user_data);
		}
		g_error_free(error);
		free(cb_container);
		return;
	}

	_icl_on_cru_cb(result, cb_container);
}


static void _icl_on_post_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	GVariant *result;
	GError *error = NULL;
	icl_on_cru_s *cb_container = user_data;

	ic_dbus_call_post_finish(IC_DBUS(object), &result, g_async_res, &error);
	if (error) {
		ERR("ic_dbus_call_post_finish() Fail(%s)", error->message);
		if (cb_container->cb) {
			int ret = icl_dbus_convert_dbus_error(error->code);
			cb_container->cb(cb_container->resource, NULL, NULL, ret,
					cb_container->user_data);
		}
		g_error_free(error);
		free(cb_container);
		return;
	}

	_icl_on_cru_cb(result, cb_container);
}


API int iotcon_remote_resource_get(iotcon_remote_resource_h resource,
		iotcon_query_h query, iotcon_remote_resource_cru_cb cb, void *user_data)
{
	GVariant *arg_query;
	GVariant *arg_remote_resource;
	icl_on_cru_s *cb_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = calloc(1, sizeof(icl_on_cru_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = resource;
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	arg_remote_resource = icl_dbus_remote_resource_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_get(icl_dbus_get_object(), arg_remote_resource, arg_query, NULL,
			_icl_on_get_cb, cb_container);

	icl_cru_cb_list = g_list_append(icl_cru_cb_list, cb_container);

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_put(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_cru_cb cb,
		void *user_data)
{
	GVariant *arg_repr;
	GVariant *arg_remote_resource;
	GVariant *arg_query;
	icl_on_cru_s *cb_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = calloc(1, sizeof(icl_on_cru_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = resource;
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	arg_repr = icl_representation_to_gvariant(repr);
	if (NULL == arg_repr) {
		ERR("icl_representation_to_gvariant() Fail");
		free(cb_container);
		return IOTCON_ERROR_REPRESENTATION;
	}

	arg_remote_resource = icl_dbus_remote_resource_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_put(icl_dbus_get_object(), arg_remote_resource, arg_repr, arg_query,
			NULL, _icl_on_put_cb, cb_container);

	icl_cru_cb_list = g_list_append(icl_cru_cb_list, cb_container);

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_post(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_cru_cb cb,
		void *user_data)
{
	GVariant *arg_repr;
	GVariant *arg_query;
	GVariant *arg_remote_resource;
	icl_on_cru_s *cb_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = calloc(1, sizeof(icl_on_cru_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = resource;
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	arg_repr = icl_representation_to_gvariant(repr);
	if (NULL == arg_repr) {
		ERR("icl_representation_to_gvariant() Fail");
		free(cb_container);
		return IOTCON_ERROR_REPRESENTATION;
	}

	arg_remote_resource = icl_dbus_remote_resource_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_post(icl_dbus_get_object(), arg_remote_resource, arg_repr, arg_query,
			NULL, _icl_on_post_cb, cb_container);

	icl_cru_cb_list = g_list_append(icl_cru_cb_list, cb_container);

	return IOTCON_ERROR_NONE;
}


static void _icl_on_delete_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	int res, ret;
	GVariant *result;
	char *option_data;
	GError *error = NULL;
	GVariantIter *options;
	unsigned short option_id;
	iotcon_options_h header_options = NULL;

	icl_on_delete_s *cb_container = user_data;
	iotcon_remote_resource_delete_cb cb = cb_container->cb;

	icl_delete_cb_list = g_list_remove(icl_delete_cb_list, cb_container);

	ic_dbus_call_delete_finish(IC_DBUS(object), &result, g_async_res, &error);
	if (error) {
		ERR("ic_dbus_call_delete_finish() Fail(%s)", error->message);
		if (cb_container->cb) {
			ret = icl_dbus_convert_dbus_error(error->code);
			cb_container->cb(cb_container->resource, NULL, ret, cb_container->user_data);
		}
		g_error_free(error);
		free(cb_container);
		return;
	}
	g_variant_get(result, "(a(qs)i)", &options, &res);

	if (IOTCON_ERROR_NONE == res && g_variant_iter_n_children(options)) {
		ret = iotcon_options_create(&header_options);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_options_create() Fail(%d)", ret);
			g_variant_iter_free(options);
			free(cb_container);
			return;
		}

		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_add(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	res = icl_dbus_convert_daemon_error(res);

	if (cb)
		cb(cb_container->resource, header_options, res, cb_container->user_data);

	if (header_options)
		iotcon_options_destroy(header_options);

	free(cb_container);
}


API int iotcon_remote_resource_delete(iotcon_remote_resource_h resource,
		iotcon_remote_resource_delete_cb cb, void *user_data)
{
	GVariant *arg_remote_resource;
	icl_on_delete_s *cb_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = calloc(1, sizeof(icl_on_delete_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = resource;
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	arg_remote_resource = icl_dbus_remote_resource_to_gvariant(resource);

	ic_dbus_call_delete(icl_dbus_get_object(), arg_remote_resource, NULL,
			_icl_on_delete_cb, cb_container);

	icl_delete_cb_list = g_list_append(icl_delete_cb_list, cb_container);

	return IOTCON_ERROR_NONE;
}


static void _icl_on_observe_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	int res, ret;
	int seq_num;
	iotcon_representation_h repr;
	GVariantIter *options;
	unsigned short option_id;
	char *option_data;
	GVariant *repr_gvar;
	iotcon_options_h header_options = NULL;

	icl_on_observe_s *cb_container = user_data;
	iotcon_remote_resource_observe_cb cb = cb_container->cb;

	g_variant_get(parameters, "(a(qs)vii)", &options, &repr_gvar, &res,
			&seq_num);

	if (IOTCON_ERROR_NONE == res && g_variant_iter_n_children(options)) {
		ret = iotcon_options_create(&header_options);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_options_create() Fail(%d)", ret);
			g_variant_iter_free(options);
			free(cb_container);
			return;
		}

		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_add(header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	repr = icl_representation_from_gvariant(repr_gvar);
	if (NULL == repr) {
		ERR("icl_representation_from_gvariant() Fail");
		if (header_options)
			iotcon_options_destroy(header_options);

		free(cb_container);
		return;
	}

	res = icl_dbus_convert_daemon_error(res);

	if (cb)
		cb(cb_container->resource, repr, header_options, res, seq_num,
				cb_container->user_data);

	if (repr)
		iotcon_representation_destroy(repr);
	if (header_options)
		iotcon_options_destroy(header_options);
}


static void _icl_observe_conn_cleanup(icl_on_observe_s *cb_container)
{
	cb_container->resource->observe_handle = 0;
	cb_container->resource->observe_sub_id = 0;
	free(cb_container);
}


int icl_remote_resource_observer_start(iotcon_remote_resource_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		GDBusSignalCallback sig_handler,
		void *cb_container,
		void *cb_free,
		unsigned int *sub_id,
		int64_t *observe_handle)
{
	GError *error = NULL;
	int ret, signal_number;
	GVariant *arg_query, *arg_remote_resource;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);

	signal_number = icl_dbus_generate_signal_number();

	arg_remote_resource = icl_dbus_remote_resource_to_gvariant(resource);
	arg_query = icl_dbus_query_to_gvariant(query);

	ic_dbus_call_observer_start_sync(icl_dbus_get_object(), arg_remote_resource,
			observe_type, arg_query, signal_number, observe_handle, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_observer_start_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		g_variant_unref(arg_query);
		g_variant_unref(arg_remote_resource);
		return ret;
	}

	if (0 == *observe_handle) {
		ERR("iotcon-daemon Fail");
		return IOTCON_ERROR_IOTIVITY;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_OBSERVE,
			signal_number);

	*sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, cb_free, sig_handler);
	if (0 == *sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_observer_start(iotcon_remote_resource_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_remote_resource_observe_cb cb,
		void *user_data)
{
	int ret;
	unsigned int sub_id;
	int64_t observe_handle;
	icl_on_observe_s *cb_container;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(resource->observe_handle || resource->observe_sub_id, IOTCON_ERROR_ALREADY);

	cb_container = calloc(1, sizeof(icl_on_observe_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->resource = resource;
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	ret = icl_remote_resource_observer_start(resource,
			observe_type,
			query,
			_icl_on_observe_cb,
			cb_container,
			_icl_observe_conn_cleanup,
			&sub_id,
			&observe_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_remote_resource_observer_start() Fail(%d)", ret);
		free(cb_container);
		return ret;
	}

	resource->observe_sub_id = sub_id;
	resource->observe_handle = observe_handle;

	return IOTCON_ERROR_NONE;
}


int icl_remote_resource_observer_stop(iotcon_remote_resource_h resource,
		iotcon_options_h options, int64_t handle, unsigned int sub_id)
{
	int ret;
	GError *error = NULL;
	GVariant *arg_options;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);

	arg_options = icl_dbus_options_to_gvariant(options);

	ic_dbus_call_observer_stop_sync(icl_dbus_get_object(), handle, arg_options,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_observer_stop_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	icl_dbus_unsubscribe_signal(resource->observe_sub_id);

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_observer_stop(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (0 == resource->observe_handle) {
		ERR("It doesn't have a observe_handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = icl_remote_resource_observer_stop(resource, resource->header_options,
			resource->observe_handle, resource->observe_sub_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_remote_resource_observer_stop() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}

