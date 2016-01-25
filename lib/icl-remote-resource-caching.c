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

#include "iotcon.h"
#include "iotcon-internal.h"
#include "ic-dbus.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-types.h"
#include "icl-representation.h"
#include "icl-list.h"
#include "icl-value.h"
#include "icl-payload.h"
#include "icl-remote-resource.h"

#include "icl-ioty.h"

typedef struct {
	iotcon_remote_resource_cached_representation_changed_cb cb;
	void *user_data;
	iotcon_remote_resource_h resource;
} icl_caching_s;


static void _icl_caching_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	int ret;
	iotcon_representation_h repr, cached_repr;
	icl_caching_s *cb_container = user_data;
	iotcon_remote_resource_cached_representation_changed_cb cb = cb_container->cb;

	repr = icl_representation_from_gvariant(parameters);

	ret = iotcon_representation_clone(repr, &cached_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_clone() Fail(%d)", ret);
		return;
	}

	if (cb_container->resource->cached_repr)
		iotcon_representation_destroy(cb_container->resource->cached_repr);
	cb_container->resource->cached_repr = cached_repr;

	if (cb) {
		cb(cb_container->resource, cb_container->resource->cached_repr,
				cb_container->user_data);
	}
}


static void _icl_caching_conn_cleanup(icl_caching_s *cb_container)
{
	if (cb_container->resource->cached_repr) {
		iotcon_representation_destroy(cb_container->resource->cached_repr);
		cb_container->resource->cached_repr = NULL;
	}
	cb_container->resource->caching_sub_id = 0;
	icl_remote_resource_unref(cb_container->resource);
	free(cb_container);
}


API int iotcon_remote_resource_start_caching(iotcon_remote_resource_h resource,
		iotcon_remote_resource_cached_representation_changed_cb cb, void *user_data)
{
	/* TEST */
	return icl_ioty_remote_resource_start_caching(resource, cb, user_data);

	int ret, sub_id;
	GError *error = NULL;
	int64_t signal_number;
	icl_caching_s *cb_container;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 != resource->caching_sub_id) {
		ERR("Already Start Caching");
		return IOTCON_ERROR_ALREADY;
	}

	INFO("Start Caching");

	ic_dbus_call_start_caching_sync(icl_dbus_get_object(),
			resource->uri_path,
			resource->host_address,
			resource->connectivity_type,
			&signal_number,
			&ret,
			NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_start_caching_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%llx", IC_DBUS_SIGNAL_CACHING,
			signal_number);

	cb_container = calloc(1, sizeof(icl_caching_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container,
			_icl_caching_conn_cleanup, _icl_caching_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		free(cb_container);
		return IOTCON_ERROR_DBUS;
	}
	resource->caching_sub_id = sub_id;
	cb_container->resource = resource;
	icl_remote_resource_ref(resource);

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_stop_caching(iotcon_remote_resource_h resource)
{
	/* TEST */
	return icl_ioty_remote_resource_stop_caching(resource);

	int ret;
	GError *error = NULL;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == resource->caching_sub_id) {
		ERR("Not Cached");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	INFO("Stop Caching");

	ic_dbus_call_stop_caching_sync(icl_dbus_get_object(),
			resource->uri_path,
			resource->host_address,
			&ret,
			NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_stop_caching_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_dbus_error(ret);
	}

	icl_dbus_unsubscribe_signal(resource->caching_sub_id);
	resource->caching_sub_id = 0;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_get_cached_representation(
		iotcon_remote_resource_h resource,
		iotcon_representation_h *representation)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == representation, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == resource->cached_repr) {
		ERR("No Caching Representation");
		return IOTCON_ERROR_NO_DATA;
	}

	*representation = resource->cached_repr;

	return IOTCON_ERROR_NONE;
}

static GHashTable *icl_caching_table;

void icl_remote_resource_caching_table_insert(iotcon_remote_resource_h resource,
		icl_caching_container_s *cb_container)
{
	RET_IF(NULL == resource);
	RET_IF(NULL == cb_container);

	if (NULL == icl_caching_table) {
		icl_caching_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
				icl_destroy_caching_container);
	}
	g_hash_table_insert(icl_caching_table, resource, cb_container);
}

int icl_remote_resource_caching_table_remove(iotcon_remote_resource_h resource)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == icl_caching_table)
		return IOTCON_ERROR_INVALID_PARAMETER;

	if (NULL == g_hash_table_lookup(icl_caching_table, resource))
		return IOTCON_ERROR_INVALID_PARAMETER;

	g_hash_table_remove(icl_caching_table, resource);

	return IOTCON_ERROR_NONE;
}

