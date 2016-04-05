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
#include <inttypes.h>
#include <errno.h>
#include <glib.h>
#include <gio/gio.h>

#include "iotcon.h"
#include "iotcon-internal.h"
#include "ic-utils.h"
#include "ic-dbus.h"
#include "icl.h"
#include "icl-options.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-representation.h"
#include "icl-response.h"
#include "icl-remote-resource.h"
#include "icl-payload.h"

#include "icl-ioty.h"

typedef struct {
	iotcon_remote_resource_response_cb cb;
	void *user_data;
	iotcon_remote_resource_h resource;
} icl_on_response_s;

typedef struct {
	iotcon_remote_resource_observe_cb cb;
	void *user_data;
	iotcon_remote_resource_h resource;
} icl_on_observe_s;

static GList *icl_crud_cb_list = NULL;

void icl_remote_resource_crud_stop(iotcon_remote_resource_h resource)
{
	GList *c;
	for (c = icl_crud_cb_list; c; c = c->next) {
		icl_on_response_s *cb_container = c->data;
		if (NULL == cb_container) {
			ERR("cb_container is NULL");
			continue;
		}
		if (cb_container->resource == resource)
			cb_container->cb = NULL;
	}
}

static iotcon_options_h _icl_parse_options_iter(GVariantIter *iter)
{
	int ret;
	iotcon_options_h options = NULL;

	if (NULL == iter)
		return NULL;

	if (g_variant_iter_n_children(iter)) {
		unsigned short option_id;
		char *option_data;

		ret = iotcon_options_create(&options);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_options_create() Fail(%d)", ret);
			return NULL;
		}
		while (g_variant_iter_loop(iter, "(q&s)", &option_id, &option_data))
			iotcon_options_add(options, option_id, option_data);
	}
	return options;
}

static int _icl_parse_crud_gvariant(iotcon_request_type_e request_type,
		GVariant *gvar, iotcon_response_h *response)
{
	int res;
	GVariantIter *options_iter = NULL;
	GVariant *repr_gvar = NULL;
	iotcon_response_h resp = NULL;
	iotcon_options_h options = NULL;
	iotcon_representation_h repr = NULL;

	if (IOTCON_REQUEST_DELETE == request_type)
		g_variant_get(gvar, "(a(qs)i)", &options_iter, &res);
	else
		g_variant_get(gvar, "(a(qs)vi)", &options_iter, &repr_gvar, &res);

	if (res < IOTCON_ERROR_NONE)
		return icl_dbus_convert_daemon_error(res);

	if (options_iter) {
		options = _icl_parse_options_iter(options_iter);
		g_variant_iter_free(options_iter);
	}

	if (repr_gvar) {
		repr = icl_representation_from_gvariant(repr_gvar);
		if (NULL == repr) {
			ERR("icl_representation_from_gvariant() Fail");
			if (options)
				iotcon_options_destroy(options);
			return IOTCON_ERROR_SYSTEM;
		}
	}

	resp = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == resp) {
		ERR("calloc() Fail(%d)", errno);
		if (repr)
			iotcon_representation_destroy(repr);
		if (options)
			iotcon_options_destroy(options);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	resp->result = res;
	resp->repr = repr;
	resp->header_options = options;

	*response = resp;

	return IOTCON_ERROR_NONE;
}

static void _icl_on_crud_cb(iotcon_request_type_e request_type,
		GObject *object, GAsyncResult *g_async_res, icl_on_response_s *cb_container)
{
	int ret;
	iotcon_response_h response = NULL;
	GVariant *result;
	GError *error = NULL;

	RET_IF(NULL == cb_container);

	icl_crud_cb_list = g_list_remove(icl_crud_cb_list, cb_container);

	switch (request_type) {
	case IOTCON_REQUEST_GET:
		ic_dbus_call_get_finish(IC_DBUS(object), &result, g_async_res, &error);
		break;
	case IOTCON_REQUEST_PUT:
		ic_dbus_call_put_finish(IC_DBUS(object), &result, g_async_res, &error);
		break;
	case IOTCON_REQUEST_POST:
		ic_dbus_call_post_finish(IC_DBUS(object), &result, g_async_res, &error);
		break;
	case IOTCON_REQUEST_DELETE:
		ic_dbus_call_delete_finish(IC_DBUS(object), &result, g_async_res, &error);
		break;
	default:
		ERR("Invalid type(%d)", request_type);
		return;
	}

	if (error) {
		ERR("ic_dbus_call_finish() Fail(%d, %s)", request_type, error->message);
		if (cb_container->cb) {
			int ret = icl_dbus_convert_dbus_error(error->code);
			cb_container->cb(cb_container->resource, ret, request_type, NULL,
					cb_container->user_data);
		}
		g_error_free(error);
		free(cb_container);
		return;
	}

	ret = _icl_parse_crud_gvariant(request_type, result, &response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_parse_crud_gvariant() Fail(%d)", ret);
		if (cb_container->cb) {
			cb_container->cb(cb_container->resource, ret, request_type, NULL,
					cb_container->user_data);
		}
		free(cb_container);
		return;
	}
	if (cb_container->cb) {
		cb_container->cb(cb_container->resource, IOTCON_ERROR_NONE, request_type,
				response, cb_container->user_data);
	}

	if (response)
		iotcon_response_destroy(response);

	free(cb_container);
}


static void _icl_on_get_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	_icl_on_crud_cb(IOTCON_REQUEST_GET, object, g_async_res, user_data);
}


static void _icl_on_put_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	_icl_on_crud_cb(IOTCON_REQUEST_PUT, object, g_async_res, user_data);
}


static void _icl_on_post_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	_icl_on_crud_cb(IOTCON_REQUEST_POST, object, g_async_res, user_data);
}


API int iotcon_remote_resource_get(iotcon_remote_resource_h resource,
		iotcon_query_h query, iotcon_remote_resource_response_cb cb, void *user_data)
{
	GVariant *arg_query;
	int ret, connectivity_type;
	GVariant *arg_remote_resource;
	icl_on_response_s *cb_container;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (true == resource->is_found) {
		ERR("The resource should be cloned.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_remote_resource_get(resource, query, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_get() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		cb_container = calloc(1, sizeof(icl_on_response_s));
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

		icl_crud_cb_list = g_list_append(icl_crud_cb_list, cb_container);
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_put(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_response_cb cb,
		void *user_data)
{
	int ret, connectivity_type;
	GVariant *arg_repr;
	GVariant *arg_remote_resource;
	GVariant *arg_query;
	icl_on_response_s *cb_container;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (true == resource->is_found) {
		ERR("The resource should be cloned.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_remote_resource_put(resource, repr, query, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_put() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		cb_container = calloc(1, sizeof(icl_on_response_s));
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

		icl_crud_cb_list = g_list_append(icl_crud_cb_list, cb_container);
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_post(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_response_cb cb,
		void *user_data)
{

	GVariant *arg_repr;
	GVariant *arg_query;
	int ret, connectivity_type;
	GVariant *arg_remote_resource;
	icl_on_response_s *cb_container;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (true == resource->is_found) {
		ERR("The resource should be cloned.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_remote_resource_post(resource, repr, query, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_post() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		cb_container = calloc(1, sizeof(icl_on_response_s));
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

		icl_crud_cb_list = g_list_append(icl_crud_cb_list, cb_container);
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


static void _icl_on_delete_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	_icl_on_crud_cb(IOTCON_REQUEST_DELETE, object, g_async_res, user_data);
}


API int iotcon_remote_resource_delete(iotcon_remote_resource_h resource,
		iotcon_remote_resource_response_cb cb, void *user_data)
{
	int ret, connectivity_type;
	GVariant *arg_remote_resource;
	icl_on_response_s *cb_container;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (true == resource->is_found) {
		ERR("The resource should be cloned.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_remote_resource_delete(resource, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_delete() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		cb_container = calloc(1, sizeof(icl_on_response_s));
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

		icl_crud_cb_list = g_list_append(icl_crud_cb_list, cb_container);
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
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
	int res;
	int seq_number = -1;
	GVariantIter *options_iter = NULL;
	GVariant *repr_gvar = NULL;
	iotcon_response_h response = NULL;
	icl_on_observe_s *cb_container = user_data;
	iotcon_options_h options = NULL;
	iotcon_representation_h repr = NULL;

	RET_IF(NULL == cb_container);

	g_variant_get(parameters, "(a(qs)vii)", &options_iter, &repr_gvar, &res, &seq_number);

	if (res < IOTCON_ERROR_NONE && cb_container->cb) {
		cb_container->cb(cb_container->resource, icl_dbus_convert_daemon_error(res),
				seq_number, NULL, cb_container->user_data);
	}

	if (options_iter) {
		options = _icl_parse_options_iter(options_iter);
		g_variant_iter_free(options_iter);
	}

	if (repr_gvar) {
		repr = icl_representation_from_gvariant(repr_gvar);
		if (NULL == repr) {
			ERR("icl_representation_from_gvariant() Fail");
			if (options)
				iotcon_options_destroy(options);

			if (cb_container->cb) {
				cb_container->cb(cb_container->resource, IOTCON_ERROR_SYSTEM, -1,
						NULL, cb_container->user_data);
			}
			return;
		}
	}

	response = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == response) {
		ERR("calloc() Fail(%d)", errno);
		if (repr)
			iotcon_representation_destroy(repr);
		if (options)
			iotcon_options_destroy(options);

		if (cb_container->cb) {
			cb_container->cb(cb_container->resource, IOTCON_ERROR_OUT_OF_MEMORY, -1,
					NULL, cb_container->user_data);
		}
		return;
	}
	response->result = res;
	response->repr = repr;
	response->header_options = options;

	if (cb_container->cb) {
		cb_container->cb(cb_container->resource, IOTCON_ERROR_NONE, seq_number,
				response, cb_container->user_data);
	}

	if (response)
		iotcon_response_destroy(response);
}


static void _icl_observe_conn_cleanup(icl_on_observe_s *cb_container)
{
	cb_container->resource->observe_handle = 0;
	cb_container->resource->observe_sub_id = 0;
	icl_remote_resource_unref(cb_container->resource);
	free(cb_container);
}


API int iotcon_remote_resource_observe_register(iotcon_remote_resource_h resource,
		iotcon_observe_policy_e observe_policy,
		iotcon_query_h query,
		iotcon_remote_resource_observe_cb cb,
		void *user_data)
{
	unsigned int sub_id;
	GError *error = NULL;
	int64_t signal_number;
	int64_t observe_handle;
	int ret, connectivity_type;
	icl_on_observe_s *cb_container;
	GVariant *arg_query, *arg_remote_resource;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(resource->observe_handle || resource->observe_sub_id, IOTCON_ERROR_ALREADY);

	if (true == resource->is_found) {
		ERR("The resource should be cloned.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_remote_resource_observe_register(resource, observe_policy, query, cb,
				user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_observe_register() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		arg_remote_resource = icl_dbus_remote_resource_to_gvariant(resource);
		arg_query = icl_dbus_query_to_gvariant(query);

		cb_container = calloc(1, sizeof(icl_on_observe_s));
		if (NULL == cb_container) {
			ERR("calloc() Fail(%d)", errno);
			g_variant_unref(arg_query);
			g_variant_unref(arg_remote_resource);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		cb_container->resource = resource;
		cb_container->cb = cb;
		cb_container->user_data = user_data;

		ic_dbus_call_observer_start_sync(icl_dbus_get_object(), arg_remote_resource,
				observe_policy, arg_query, &signal_number, &observe_handle, NULL, &error);
		if (error) {
			ERR("ic_dbus_call_observer_start_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			g_variant_unref(arg_query);
			g_variant_unref(arg_remote_resource);
			free(cb_container);
			return ret;
		}
		if (0 == observe_handle) {
			ERR("iotcon-daemon Fail");
			free(cb_container);
			return IOTCON_ERROR_IOTIVITY;
		}

		snprintf(signal_name, sizeof(signal_name), "%s_%"PRIx64, IC_DBUS_SIGNAL_OBSERVE,
				signal_number);

		sub_id = icl_dbus_subscribe_signal(signal_name, cb_container,
				_icl_observe_conn_cleanup, _icl_on_observe_cb);
		if (0 == sub_id) {
			ERR("icl_dbus_subscribe_signal() Fail");
			free(cb_container);
			return IOTCON_ERROR_DBUS;
		}

		resource->observe_sub_id = sub_id;
		resource->observe_handle = observe_handle;
		icl_remote_resource_ref(resource);
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_observe_deregister(iotcon_remote_resource_h resource)
{
	GError *error = NULL;
	GVariant *arg_options;
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (true == resource->is_found) {
		ERR("The resource should be cloned.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if (0 == resource->observe_handle) {
		ERR("It doesn't have a observe_handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_remote_resource_observe_deregister(resource);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_observe_deregister() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		arg_options = icl_dbus_options_to_gvariant(resource->header_options);

		ic_dbus_call_observer_stop_sync(icl_dbus_get_object(), resource->observe_handle,
				arg_options, &ret, NULL, &error);
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
		resource->observe_handle = 0;

		icl_dbus_unsubscribe_signal(resource->observe_sub_id);
		resource->observe_sub_id = 0;
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

