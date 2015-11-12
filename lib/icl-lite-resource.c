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
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-dbus.h"
#include "icl-representation.h"
#include "icl-state.h"
#include "icl-value.h"
#include "icl-list.h"
#include "icl-dbus-type.h"
#include "icl-payload.h"
#include "icl-resource.h"
#include "icl-response.h"

struct icl_lite_resource {
	char *uri_path;
	iotcon_state_h state;
	int64_t handle;
	unsigned int sub_id;
	int properties;
};


static inline int _icl_lite_resource_set_state(iotcon_state_h state,
		iotcon_state_h res_state)
{
	GHashTableIter iter;
	gpointer key, value;
	iotcon_value_h res_value, src_value;

	g_hash_table_iter_init(&iter, state->hash_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		res_value = g_hash_table_lookup(res_state->hash_table, key);
		if (NULL == res_value) {
			WARN("Invalid Key(%s)", key);
			continue;
		}

		src_value = icl_value_clone(value);
		if (NULL == src_value) {
			ERR("icl_value_clone() Fail");
			return IOTCON_ERROR_REPRESENTATION;
		}

		if (src_value->type != res_value->type) {
			WARN("Different Type(%d)", (src_value->type));
			icl_value_destroy(src_value);
			continue;
		}

		g_hash_table_replace(res_state->hash_table, ic_utils_strdup(key), src_value);
	}

	return IOTCON_ERROR_NONE;
}


static int _icl_lite_resource_response_send(iotcon_representation_h repr,
		int64_t oic_request_h, int64_t oic_resource_h, int response_result)
{
	int ret;
	iotcon_response_h response;

	response = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == response) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	response->iface = IOTCON_INTERFACE_DEFAULT;
	response->result = response_result;
	response->oic_request_h = oic_request_h;
	response->oic_resource_h = oic_resource_h;
	response->repr = repr;
	icl_representation_inc_ref_count(response->repr);

	ret = iotcon_response_send(response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_send() Fail(%d)", ret);
		iotcon_response_destroy(response);
		return ret;
	}

	iotcon_response_destroy(response);

	return IOTCON_ERROR_NONE;
}


static int _icl_lite_resource_notify(iotcon_lite_resource_h lite_resource)
{
	int ret;
	struct icl_resource resource = {0};

	RETV_IF(NULL == lite_resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (false == (IOTCON_OBSERVABLE & lite_resource->properties))
		return IOTCON_ERROR_NONE;

	resource.handle = lite_resource->handle;
	resource.sub_id = lite_resource->sub_id;

	ret = iotcon_resource_notify(&resource, NULL, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_notify() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


static void _icl_lite_resource_request_handler(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	GVariant *repr_gvar;
	int ret, request_type;
	iotcon_representation_h repr;
	iotcon_state_h recv_state = NULL;
	GVariantIter *repr_iter, *state_iter;
	int64_t oic_request_h, oic_resource_h;
	iotcon_lite_resource_h resource = user_data;

	ret = iotcon_representation_create(&repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		_icl_lite_resource_response_send(NULL, oic_request_h, oic_resource_h,
				IOTCON_RESPONSE_RESULT_ERROR);
		return;
	}

	ret = iotcon_representation_set_uri_path(repr, resource->uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_uri_path() Fail(%d)", ret);
		_icl_lite_resource_response_send(repr, oic_request_h, oic_resource_h,
				IOTCON_RESPONSE_RESULT_ERROR);
		iotcon_representation_destroy(repr);
		return;
	}

	g_variant_get(parameters, "(siia(qs)a(ss)iiavxx)",
			NULL,	/* host address */
			NULL,	/* connectivity type */
			&request_type,
			NULL,	/* header options */
			NULL,	/* query */
			NULL,	/* observe action */
			NULL,	/* observe_id */
			&repr_iter,
			&oic_request_h,
			&oic_resource_h);

	switch (request_type) {
	case IOTCON_REQUEST_GET:
		break;
	case IOTCON_REQUEST_PUT:
		if (FALSE == g_variant_iter_loop(repr_iter, "v", &repr_gvar)) {
			ERR("Received representation is empty.");
			_icl_lite_resource_response_send(repr, oic_request_h, oic_resource_h,
					IOTCON_RESPONSE_RESULT_ERROR);
			iotcon_representation_destroy(repr);
			return;
		}

		g_variant_get(repr_gvar, "(siasa{sv}av)", NULL, NULL, NULL, &state_iter,
				NULL);

		ret = iotcon_state_create(&recv_state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_create() Fail(%d)", ret);
			_icl_lite_resource_response_send(repr, oic_request_h, oic_resource_h,
					IOTCON_RESPONSE_RESULT_ERROR);
			iotcon_representation_destroy(repr);
			return;
		}

		icl_state_from_gvariant(recv_state, state_iter);

		ret = _icl_lite_resource_set_state(recv_state, resource->state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_lite_resource_set_state() Fail(%d)", ret);
			_icl_lite_resource_response_send(repr, oic_request_h, oic_resource_h,
					IOTCON_RESPONSE_RESULT_ERROR);
			iotcon_state_destroy(recv_state);
			iotcon_representation_destroy(repr);
			return;
		}

		iotcon_state_destroy(recv_state);
		break;
	case IOTCON_REQUEST_POST:
	case IOTCON_REQUEST_DELETE:
	default:
		WARN("Not supported request (only GET / PUT / OBSERVE)");
		ret = _icl_lite_resource_response_send(repr, oic_request_h, oic_resource_h,
				IOTCON_RESPONSE_RESULT_FORBIDDEN);
		if (IOTCON_ERROR_NONE != ret)
			ERR("_icl_lite_resource_response_send() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return;
	}

	ret = iotcon_representation_set_state(repr, resource->state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		_icl_lite_resource_response_send(repr, oic_request_h, oic_resource_h,
				IOTCON_RESPONSE_RESULT_ERROR);
		iotcon_representation_destroy(repr);
		return;
	}

	ret = _icl_lite_resource_response_send(repr, oic_request_h, oic_resource_h,
			IOTCON_RESPONSE_RESULT_OK);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_lite_resource_response_send() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return;
	}

	if (IOTCON_REQUEST_PUT == request_type) {
		ret = _icl_lite_resource_notify(resource);
		if (IOTCON_ERROR_NONE != ret)
			WARN("_icl_lite_resource_notify() Fail(%d)", ret);
	}

	iotcon_representation_destroy(repr);
}


static void _icl_lite_resource_conn_cleanup(iotcon_lite_resource_h resource)
{
	resource->sub_id = 0;

	if (resource->handle) {
		resource->handle = 0;
		return;
	}

	iotcon_state_destroy(resource->state);
	free(resource->uri_path);
	free(resource);
}


/* The length of uri_path should be less than or equal to 36. */
API int iotcon_lite_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		int properties,
		iotcon_state_h state,
		iotcon_lite_resource_h *resource_handle)
{
	int ret, iface;
	const gchar **types;
	GError *error = NULL;
	iotcon_lite_resource_h resource;
	unsigned int sub_id, signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH];

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(ICL_URI_PATH_LENGTH_MAX < strlen(uri_path),
			IOTCON_ERROR_INVALID_PARAMETER, "Invalid uri_path(%s)", uri_path);
	RETV_IF(NULL == res_types, IOTCON_ERROR_INVALID_PARAMETER);

	resource = calloc(1, sizeof(struct icl_lite_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	iface = IOTCON_INTERFACE_DEFAULT;

	types = icl_dbus_resource_types_to_array(res_types);
	if (NULL == types) {
		ERR("icl_dbus_resource_types_to_array() Fail");
		free(resource);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	signal_number = icl_dbus_generate_signal_number();

	ic_dbus_call_register_resource_sync(icl_dbus_get_object(), uri_path, types, iface,
			properties, signal_number, &(resource->handle), NULL, &error);
	if (error) {
		ERR("ic_dbus_call_register_resource_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		free(types);
		free(resource);
		return ret;
	}
	free(types);

	if (0 == resource->handle) {
		ERR("iotcon-daemon Fail");
		free(resource);
		return IOTCON_ERROR_IOTIVITY;
	}

	resource->properties = properties;
	resource->uri_path = ic_utils_strdup(uri_path);

	ret = icl_state_clone(state, &(resource->state));
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_state_clone() Fail(%d)", ret);
		free(resource->uri_path);
		free(resource);
		return ret;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_REQUEST_HANDLER,
			signal_number);

	sub_id = icl_dbus_subscribe_signal(signal_name, resource,
			_icl_lite_resource_conn_cleanup, _icl_lite_resource_request_handler);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		iotcon_state_destroy(resource->state);
		free(resource->uri_path);
		free(resource);
		return IOTCON_ERROR_DBUS;
	}

	resource->sub_id = sub_id;

	*resource_handle = resource;

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_destroy(iotcon_lite_resource_h resource)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == resource->handle) { /* disconnected iotcon dbus */
		WARN("Invalid Resource handle");
		iotcon_state_destroy(resource->state);
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

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_update_state(iotcon_lite_resource_h resource,
		iotcon_state_h state)
{
	int ret;
	iotcon_state_h cloned_state = NULL;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_clone(state, &cloned_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_state_clone() Fail(%d)", ret);
		return ret;
	}

	iotcon_state_destroy(resource->state);

	resource->state = cloned_state;

	ret = _icl_lite_resource_notify(resource);
	if (IOTCON_ERROR_NONE != ret)
		WARN("_icl_lite_resource_notify() Fail");

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_get_state(iotcon_lite_resource_h resource,
		iotcon_state_h *state)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	*state = resource->state;

	return IOTCON_ERROR_NONE;
}
