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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <glib.h>
#include <tizen_type.h>

#include "iotcon.h"
#include "iotcon-internal.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-representation.h"
#include "icl-dbus.h"
#include "icl-request.h"
#include "icl-dbus-type.h"
#include "icl-resource-types.h"
#include "icl-resource-interfaces.h"
#include "icl-resource.h"
#include "icl-payload.h"

#include "icl-ioty.h"

static void _icl_request_handler(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	int ret;
	char *key = NULL;
	char *option_data;
	char *value = NULL;
	GVariant *repr_gvar;
	GVariantIter *query;
	GVariantIter *options;
	GVariantIter *repr_iter;
	unsigned short option_id;
	struct icl_resource_request request = {0};
	iotcon_resource_h resource = user_data;
	iotcon_request_handler_cb cb = resource->cb;

	g_variant_get(parameters, "(siia(qs)a(ss)iiavxx)",
			&request.host_address,
			&request.connectivity_type,
			&request.type,
			&options,
			&query,
			&request.observation_info.action,
			&request.observation_info.observe_id,
			&repr_iter,
			&request.oic_request_h,
			&request.oic_resource_h);

	if (g_variant_iter_n_children(options)) {
		ret = iotcon_options_create(&request.header_options);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_options_create() Fail(%d)", ret);
			g_variant_iter_free(options);
			g_variant_iter_free(query);
			g_variant_iter_free(repr_iter);
			return;
		}

		while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data))
			iotcon_options_add(request.header_options, option_id, option_data);
	}
	g_variant_iter_free(options);

	if (g_variant_iter_n_children(query)) {
		ret = iotcon_query_create(&request.query);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_query_create() Fail(%d)", ret);
			g_variant_iter_free(query);
			g_variant_iter_free(repr_iter);
			if (request.header_options)
				iotcon_options_destroy(request.header_options);
			return;
		}

		while (g_variant_iter_loop(query, "(&s&s)", &key, &value))
			iotcon_query_add(request.query, key, value);
	}
	g_variant_iter_free(query);

	if (g_variant_iter_loop(repr_iter, "v", &repr_gvar)) {
		request.repr = icl_representation_from_gvariant(repr_gvar);
		if (NULL == request.repr) {
			ERR("icl_representation_from_gvariant() Fail");
			if (request.query)
				iotcon_query_destroy(request.query);
			if (request.header_options)
				iotcon_options_destroy(request.header_options);
			return;
		}
	}
	g_variant_iter_free(repr_iter);

	/* for iotcon_resource_notify */
	if (IOTCON_OBSERVE_REGISTER == request.observation_info.action) {
		if (NULL == resource->observers)
			iotcon_observers_create(&resource->observers);
		iotcon_observers_add(resource->observers, request.observation_info.observe_id);
	} else if (IOTCON_OBSERVE_DEREGISTER == request.observation_info.action) {
		iotcon_observers_remove(resource->observers, request.observation_info.observe_id);
	}

	if (cb)
		cb(resource, &request, resource->user_data);

	/* To avoid unnecessary ERR log (representation could be NULL) */
	if (request.repr)
		iotcon_representation_destroy(request.repr);
	if (request.query)
		iotcon_query_destroy(request.query);
	if (request.header_options)
		iotcon_options_destroy(request.header_options);
}


static void _icl_resource_conn_cleanup(iotcon_resource_h resource)
{
	resource->sub_id = 0;

	if (resource->handle) {
		resource->handle = 0;
		return;
	}

	iotcon_resource_interfaces_destroy(resource->ifaces);
	iotcon_resource_types_destroy(resource->types);
	if (resource->observers)
		iotcon_observers_destroy(resource->observers);
	free(resource->uri_path);
	free(resource);
}


/* The length of uri_path should be less than or equal to 36. */
API int iotcon_resource_create2(const char *uri_path,
		iotcon_resource_types_h res_types,
		iotcon_resource_interfaces_h ifaces,
		int properties,
		iotcon_request_handler_cb cb,
		void *user_data,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_resource_h *resource_handle)
{
	int ret, conn_type;
	unsigned int sub_id;
	const gchar **type_array, **iface_array;
	GError *error = NULL;
	iotcon_resource_h resource;
	int64_t signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH];

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(ICL_URI_PATH_LENGTH_MAX < strlen(uri_path),
			IOTCON_ERROR_INVALID_PARAMETER, "Invalid uri_path(%s)", uri_path);
	RETV_IF(NULL == res_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_handle, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_check_connectivity_type(connectivity_type, icl_get_service_mode());
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_check_connectivity_type() Fail(%d)", ret);
		return ret;
	}

	conn_type = connectivity_type;

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_create(uri_path, res_types, ifaces, properties, cb,
				user_data, resource_handle);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_create() Fail(%d)", ret);
			return ret;
		}
		(*resource_handle)->connectivity_type = connectivity_type;
		break;
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		resource = calloc(1, sizeof(struct icl_resource));
		if (NULL == resource) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		type_array = icl_dbus_resource_types_to_array(res_types);
		if (NULL == type_array) {
			ERR("icl_dbus_resource_types_to_array() Fail");
			free(resource);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		iface_array = icl_dbus_resource_interfaces_to_array(ifaces);
		if (NULL == iface_array) {
			ERR("icl_dbus_resource_interfaces_to_array() Fail");
			free(type_array);
			free(resource);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		ic_dbus_call_register_resource_sync(icl_dbus_get_object(), uri_path, type_array, iface_array,
				properties, false, &signal_number, &(resource->handle), NULL, &error);
		if (error) {
			ERR("ic_dbus_call_register_resource_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			free(iface_array);
			free(type_array);
			free(resource);
			return ret;
		}
		free(iface_array);
		free(type_array);

		if (0 == resource->handle) {
			ERR("iotcon-daemon Fail");
			free(resource);
			return IOTCON_ERROR_IOTIVITY;
		}

		resource->cb = cb;
		resource->user_data = user_data;

		resource->types = icl_resource_types_ref(res_types);
		resource->uri_path = ic_utils_strdup(uri_path);
		resource->ifaces = icl_resource_interfaces_ref(ifaces);
		resource->properties = properties;
		resource->connectivity_type = connectivity_type;

		snprintf(signal_name, sizeof(signal_name), "%s_%"PRIx64,
				IC_DBUS_SIGNAL_REQUEST_HANDLER, signal_number);

		sub_id = icl_dbus_subscribe_signal(signal_name, resource, _icl_resource_conn_cleanup,
				_icl_request_handler);
		if (0 == sub_id) {
			ERR("icl_dbus_subscribe_signal() Fail");
			iotcon_resource_interfaces_destroy(resource->ifaces);
			iotcon_resource_types_destroy(res_types);
			free(resource->uri_path);
			free(resource);
			return IOTCON_ERROR_DBUS;
		}

		resource->sub_id = sub_id;

		*resource_handle = resource;
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", conn_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


/* The length of uri_path should be less than or equal to 36. */
API int iotcon_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		iotcon_resource_interfaces_h ifaces,
		int properties,
		iotcon_request_handler_cb cb,
		void *user_data,
		iotcon_resource_h *resource_handle)
{
	int ret;
	iotcon_connectivity_type_e connectivity_type = IOTCON_CONNECTIVITY_ALL;

	ret = iotcon_resource_create2(uri_path, res_types, ifaces, properties,
			cb, user_data, connectivity_type, resource_handle);

	return ret;
}


API int iotcon_resource_destroy(iotcon_resource_h resource)
{
	FN_CALL;
	GError *error = NULL;
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_destroy(resource);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_destroy() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_ALL:
		if (0 == resource->handle) { /* iotcon dbus disconnected */
			WARN("Invalid Resource handle");
			iotcon_resource_interfaces_destroy(resource->ifaces);
			iotcon_resource_types_destroy(resource->types);
			if (resource->observers)
				iotcon_observers_destroy(resource->observers);
			free(resource->uri_path);
			free(resource);
			return IOTCON_ERROR_NONE;
		}

		if (NULL == icl_dbus_get_object()) {
			ERR("icl_dbus_get_object() return NULL");
			return IOTCON_ERROR_DBUS;
		}

		ic_dbus_call_unregister_resource_sync(icl_dbus_get_object(), resource->handle, NULL,
				&error);
		if (error) {
			ERR("ic_dbus_call_unregister_resource_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			return ret;
		}
		resource->handle = 0;

		icl_dbus_unsubscribe_signal(resource->sub_id);
		resource->sub_id = 0;
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_bind_interface(iotcon_resource_h resource, const char *iface)
{
	FN_CALL;
	GError *error = NULL;
	int ret, connectivity_type;
	iotcon_resource_interfaces_h resource_ifaces;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == iface, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_bind_interface(resource, iface);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_bind_interface() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		if (0 == resource->sub_id) {
			ERR("Invalid Resource handle");
			return IOTCON_ERROR_INVALID_PARAMETER;
		}

		ret = iotcon_resource_interfaces_clone(resource->ifaces, &resource_ifaces);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_interfaces_clone() Fail(%d)", ret);
			return ret;
		}

		ret = iotcon_resource_interfaces_add(resource_ifaces, iface);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_interfaces_add() Fail(%d)", ret);
			iotcon_resource_interfaces_destroy(resource_ifaces);
			return ret;
		}

		ic_dbus_call_bind_interface_sync(icl_dbus_get_object(), resource->handle,
				iface, &ret, NULL, &error);
		if (error) {
			ERR("ic_dbus_call_bind_interface_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			iotcon_resource_interfaces_destroy(resource_ifaces);
			return ret;
		}

		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon-daemon Fail(%d)", ret);
			iotcon_resource_interfaces_destroy(resource_ifaces);
			return icl_dbus_convert_daemon_error(ret);
		}

		iotcon_resource_interfaces_destroy(resource->ifaces);
		resource->ifaces = resource_ifaces;

		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_bind_type(iotcon_resource_h resource, const char *resource_type)
{
	FN_CALL;
	GError *error = NULL;
	int ret, connectivity_type;
	iotcon_resource_types_h resource_types;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_type, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_bind_type(resource, resource_type);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_bind_type() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		if (0 == resource->sub_id) {
			ERR("Invalid Resource handle");
			return IOTCON_ERROR_INVALID_PARAMETER;
		}

		ret = iotcon_resource_types_clone(resource->types, &resource_types);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_types_clone() Fail(%d)", ret);
			return ret;
		}

		ret = iotcon_resource_types_add(resource_types, resource_type);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_types_add() Fail(%d)", ret);
			iotcon_resource_types_destroy(resource_types);
			return ret;
		}

		ic_dbus_call_bind_type_sync(icl_dbus_get_object(), resource->handle, resource_type,
				&ret, NULL, &error);
		if (error) {
			ERR("ic_dbus_call_bind_type_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			iotcon_resource_types_destroy(resource_types);
			return ret;
		}

		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon-daemon Fail(%d)", ret);
			iotcon_resource_types_destroy(resource_types);
			return icl_dbus_convert_daemon_error(ret);
		}

		iotcon_resource_types_destroy(resource->types);
		resource->types = resource_types;
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_resource_set_request_handler(iotcon_resource_h resource,
		iotcon_request_handler_cb cb, void *user_data)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	DBG("Request handler is changed");
	resource->cb = cb;
	resource->user_data = user_data;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_bind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child)
{
	GError *error = NULL;
	int i, ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(parent == child, IOTCON_ERROR_INVALID_PARAMETER);

	for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
		if (child == parent->children[i]) {
			ERR("Child resource was already bound to parent resource.");
			return IOTCON_ERROR_ALREADY;
		}
	}

	connectivity_type = parent->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_bind_child_resource(parent, child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_bind_child_resource() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		if (0 == parent->sub_id) {
			ERR("Invalid Resource handle(parent)");
			return IOTCON_ERROR_INVALID_PARAMETER;
		}
		if (0 == child->sub_id) {
			ERR("Invalid Resource handle(child)");
			return IOTCON_ERROR_INVALID_PARAMETER;
		}

		for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
			if (NULL == parent->children[i]) {
				ic_dbus_call_bind_resource_sync(icl_dbus_get_object(), parent->handle,
						child->handle, &ret, NULL, &error);
				if (error) {
					ERR("ic_dbus_call_bind_resource_sync() Fail(%s)", error->message);
					ret = icl_dbus_convert_dbus_error(error->code);
					g_error_free(error);
					return ret;
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
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_unbind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child)
{
	GError *error = NULL;
	int i, ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = parent->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_unbind_child_resource(parent, child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_unbind_child_resource() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
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
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			return ret;
		}

		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon-daemon Fail(%d)", ret);
			return icl_dbus_convert_daemon_error(ret);
		}

		for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
			if (child == parent->children[i])
				parent->children[i] = NULL;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_number_of_children(iotcon_resource_h resource, int *number)
{
	int i;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == number, IOTCON_ERROR_INVALID_PARAMETER);

	*number = 0;
	for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
		if (resource->children[i])
			*number += 1;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_nth_child(iotcon_resource_h parent, int index,
		iotcon_resource_h *child)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);
	if ((index < 0) || (ICL_CONTAINED_RESOURCES_MAX <= index)) {
		ERR("Invalid index(%d)", index);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*child = parent->children[index];

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_resource_get_uri_path(iotcon_resource_h resource, char **uri_path)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = resource->uri_path;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_resource_get_types(iotcon_resource_h resource,
		iotcon_resource_types_h *types)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = resource->types;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_interfaces(iotcon_resource_h resource,
		iotcon_resource_interfaces_h *ifaces)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = resource->ifaces;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_properties(iotcon_resource_h resource, int *properties)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == properties, IOTCON_ERROR_INVALID_PARAMETER);

	*properties = resource->properties;

	return IOTCON_ERROR_NONE;
}

API int iotcon_resource_notify(iotcon_resource_h resource,
		iotcon_representation_h repr, iotcon_observers_h observers, iotcon_qos_e qos)
{
	GError *error = NULL;
	GVariant *obs;
	GVariant *repr_gvar;
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_notify(resource, repr, observers, qos);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_notify() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		if (0 == resource->sub_id) {
			ERR("Invalid Resource handle");
			return IOTCON_ERROR_INVALID_PARAMETER;
		}

		repr_gvar = icl_dbus_representation_to_gvariant(repr);
		if (NULL == repr_gvar) {
			ERR("icl_representation_to_gvariant() Fail");
			return IOTCON_ERROR_SYSTEM;
		}

		if (observers)
			obs = icl_dbus_observers_to_gvariant(observers);
		else
			obs = icl_dbus_observers_to_gvariant(resource->observers);

		ic_dbus_call_notify_sync(icl_dbus_get_object(), resource->handle, repr_gvar, obs, qos,
				&ret, NULL, &error);
		if (error) {
			ERR("ic_dbus_call_notify_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			g_variant_unref(obs);
			g_variant_unref(repr_gvar);
			return ret;
		}

		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon-daemon Fail(%d)", ret);
			return icl_dbus_convert_daemon_error(ret);
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

