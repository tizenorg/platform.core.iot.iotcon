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
#include "iotcon-internal.h"
#include "icl.h"
#include "icl-types.h"
#include "icl-dbus.h"
#include "ic-utils.h"
#include "icl-remote-resource.h"

#include "icl-ioty.h"


typedef struct {
	iotcon_remote_resource_state_changed_cb cb;
	void *user_data;
	iotcon_remote_resource_h resource;
} icl_monitoring_s;

static void _icl_monitoring_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	icl_monitoring_s *cb_container = user_data;
	iotcon_remote_resource_state_e resource_state;
	iotcon_remote_resource_state_changed_cb cb = cb_container->cb;

	g_variant_get(parameters, "(i)", &resource_state);

	if (cb)
		cb(cb_container->resource, resource_state, cb_container->user_data);

}


static void _icl_monitoring_conn_cleanup(icl_monitoring_s *cb_container)
{
	cb_container->resource->monitoring_sub_id = 0;
	icl_remote_resource_unref(cb_container->resource);
	free(cb_container);
}


API int iotcon_remote_resource_start_monitoring(iotcon_remote_resource_h resource,
		iotcon_remote_resource_state_changed_cb cb, void *user_data)
{
	int ret, sub_id;
	GError *error = NULL;
	int64_t signal_number;
	icl_monitoring_s *cb_container;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	iotcon_service_mode_e mode;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	mode = icl_get_service_mode();

	switch (mode) {
	case IOTCON_SERVICE_WIFI:
		ret = icl_ioty_remote_resource_start_monitoring(resource, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_start_monitoring() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_SERVICE_BT:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		if (0 != resource->monitoring_sub_id) {
			ERR("Already Start Monitoring");
			return IOTCON_ERROR_ALREADY;
		}

		ic_dbus_call_start_monitoring_sync(icl_dbus_get_object(),
				resource->uri_path,
				resource->host_address,
				resource->connectivity_type,
				&signal_number,
				&ret,
				NULL,
				&error);
		if (error) {
			ERR("ic_dbus_call_start_monitoring_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			return IOTCON_ERROR_DBUS;
		}
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon-daemon Fail(%d)", ret);
			return icl_dbus_convert_daemon_error(ret);
		}

		snprintf(signal_name, sizeof(signal_name), "%s_%llx", IC_DBUS_SIGNAL_MONITORING,
				signal_number);

		cb_container = calloc(1, sizeof(icl_monitoring_s));
		if (NULL == cb_container) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		cb_container->cb = cb;
		cb_container->user_data = user_data;

		sub_id = icl_dbus_subscribe_signal(signal_name, cb_container,
				_icl_monitoring_conn_cleanup, _icl_monitoring_cb);
		if (0 == sub_id) {
			ERR("icl_dbus_subscribe_signal() Fail");
			free(cb_container);
			return IOTCON_ERROR_DBUS;
		}
		resource->monitoring_sub_id = sub_id;
		cb_container->resource = resource;
		icl_remote_resource_ref(resource);
		break;
	default:
		ERR("Invalid mode(%d)", mode);
		return IOTCON_ERROR_SYSTEM; /* TODO : Error not connected? */
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_stop_monitoring(iotcon_remote_resource_h resource)
{
	int ret;
	GError *error = NULL;
	iotcon_service_mode_e mode;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	INFO("Stop Monitoring");

	mode = icl_get_service_mode();
	switch (mode) {
	case IOTCON_SERVICE_WIFI:
		ret = icl_ioty_remote_resource_stop_monitoring(resource);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_stop_monitoring() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_SERVICE_BT:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		if (0 == resource->monitoring_sub_id) {
			ERR("Not Monitoring");
			return IOTCON_ERROR_INVALID_PARAMETER;
		}

		ic_dbus_call_stop_monitoring_sync(icl_dbus_get_object(),
				resource->uri_path,
				resource->host_address,
				&ret,
				NULL,
				&error);
		if (error) {
			ERR("ic_dbus_call_stop_monitoring_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			return ret;
		}
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon-daemon Fail(%d)", ret);
			return icl_dbus_convert_daemon_error(ret);
		}

		icl_dbus_unsubscribe_signal(resource->monitoring_sub_id);
		resource->monitoring_sub_id = 0;
		break;
	default:
		ERR("Invalid mode(%d)", mode);
		return IOTCON_ERROR_SYSTEM; /* TODO : Error not connected? */
	}

	return IOTCON_ERROR_NONE;
}



