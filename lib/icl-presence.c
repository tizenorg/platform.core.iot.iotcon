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
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "iotcon-internal.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource-types.h"
#include "icl-dbus.h"

#define ICL_PRESENCE_TTL_SECONDS_MAX (60 * 60 * 24) /* 60 sec/min * 60 min/hr * 24 hr/day */

typedef struct icl_presence {
	iotcon_presence_cb cb;
	void *user_data;
	unsigned int id;
	int64_t handle;
} icl_presence_s;

API int iotcon_start_presence(unsigned int time_to_live)
{
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(ICL_PRESENCE_TTL_SECONDS_MAX < time_to_live, IOTCON_ERROR_INVALID_PARAMETER);

	ic_dbus_call_start_presence_sync(icl_dbus_get_object(), time_to_live, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_start_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_stop_presence(void)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);

	ic_dbus_call_stop_presence_sync(icl_dbus_get_object(), &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_stop_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}


static void _icl_presence_cb(GDBusConnection *connection,
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
	icl_presence_s *presence_container = user_data;
	iotcon_presence_cb cb = presence_container->cb;

	g_variant_get(parameters, "(iu&s)", &res, &nonce, &host_address);

	res = icl_dbus_convert_daemon_error(res);

	if (cb)
		cb(res, nonce, host_address, presence_container->user_data);
}


static void _icl_presence_conn_cleanup(icl_presence_s *presence)
{
	presence->id = 0;

	if (presence->handle) {
		presence->handle = 0;
		return;
	}

	free(presence);
}


/* The length of resource_type should be less than or equal to 61. */
API int iotcon_subscribe_presence(const char *host_address, const char *resource_type,
		iotcon_presence_cb cb, void *user_data, iotcon_presence_h *presence_handle)
{
	FN_CALL;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number, ret;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	icl_presence_s *presence_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (resource_type && (ICL_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type))) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	signal_number = icl_dbus_generate_signal_number();

	presence_container = calloc(1, sizeof(icl_presence_s));
	if (NULL == presence_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	resource_type = ic_utils_dbus_encode_str(resource_type);

	ic_dbus_call_subscribe_presence_sync(icl_dbus_get_object(), host_address,
			resource_type, signal_number, &(presence_container->handle), NULL, &error);
	if (error) {
		ERR("ic_dbus_call_subscribe_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		free(presence_container);
		return ret;
	}

	if (0 == presence_container->handle) {
		ERR("iotcon-daemon Fail");
		free(presence_container);
		return IOTCON_ERROR_IOTIVITY;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_PRESENCE,
			signal_number);

	presence_container->cb = cb;
	presence_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, presence_container,
			_icl_presence_conn_cleanup, _icl_presence_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		free(presence_container);
		return IOTCON_ERROR_DBUS;
	}

	presence_container->id = sub_id;

	*presence_handle = presence_container;

	return IOTCON_ERROR_NONE;
}


API int iotcon_unsubscribe_presence(iotcon_presence_h presence)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == presence, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == presence->id) {
		WARN("Invalid Presence handle");
		free(presence);
		return IOTCON_ERROR_NONE;
	}

	ic_dbus_call_unsubscribe_presence_sync(icl_dbus_get_object(), presence->handle,
			&ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_unsubscribe_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}
	presence->handle = 0;

	icl_dbus_unsubscribe_signal(presence->id);

	return IOTCON_ERROR_NONE;
}

