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
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-repr.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"

/**
 * @brief The maximum length which can be held in a manufacturer name.
 *
 * @since_tizen 3.0
 */
#define ICL_MANUFACTURER_NAME_LENGTH_MAX 15

/**
 * @brief The maximum length which can be held in a manufacturer url.
 *
 * @since_tizen 3.0
 */
#define ICL_MANUFACTURER_URL_LENGTH_MAX 32


typedef struct {
	iotcon_device_info_cb cb;
	void *user_data;
	unsigned int id;
} icl_device_info_s;

typedef struct {
	iotcon_platform_info_cb cb;
	void *user_data;
	unsigned int id;
} icl_platform_info_s;


API int iotcon_register_device_info(const char *device_name)
{
	int ret;
	GError *error = NULL;
	GVariant *arg_info;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == device_name, IOTCON_ERROR_INVALID_PARAMETER);

	arg_info = icl_dbus_device_info_to_gvariant(device_name);
	ic_dbus_call_register_device_info_sync(icl_dbus_get_object(), arg_info, &ret,
			NULL, &error);
	if (error) {
		ERR("ic_dbus_call_register_device_info_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_info);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return ret;
}


static void _icl_device_info_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	icl_device_info_s *cb_container = user_data;
	iotcon_device_info_cb cb = cb_container->cb;
	char *device_name, *sid, *spec_version, *data_model_version;

	g_variant_get(parameters, "(&s&s&s&s)", &device_name, &sid, &spec_version,
			&data_model_version);

	if (cb)
		cb(device_name, sid, spec_version, data_model_version, cb_container->user_data);
}


API int iotcon_get_device_info(const char *host_address, iotcon_device_info_cb cb,
		void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	icl_device_info_s *cb_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	signal_number = icl_dbus_generate_signal_number();

	ic_dbus_call_get_device_info_sync(icl_dbus_get_object(), host_address,
			signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_get_device_info_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_DEVICE,
			signal_number);

	cb_container = calloc(1, sizeof(icl_device_info_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, free,
			_icl_device_info_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}


/* The length of manufacturer_name should be less than and equal to 16.
 * The length of manufacturer_url should be less than and equal to 32. */
API int iotcon_register_platform_info(iotcon_platform_info_s *platform_info)
{
	int ret;
	GError *error = NULL;
	GVariant *arg_info;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);

	if (NULL == platform_info->platform_id
			|| NULL == platform_info->manuf_name
			|| NULL == platform_info->manuf_url
			|| NULL == platform_info->model_number
			|| NULL == platform_info->date_of_manufacture
			|| NULL == platform_info->platform_ver
			|| NULL == platform_info->os_ver
			|| NULL == platform_info->hardware_ver
			|| NULL == platform_info->firmware_ver
			|| NULL == platform_info->support_url
			|| NULL == platform_info->system_time) {
		ERR("one of parameter is NULL");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if (platform_info->manuf_name
			&& (ICL_MANUFACTURER_NAME_LENGTH_MAX < strlen(platform_info->manuf_name))) {
		ERR("The length of manufacturer_name(%s) is invalid.", platform_info->manuf_name);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if (platform_info->manuf_url
			&& (ICL_MANUFACTURER_URL_LENGTH_MAX < strlen(platform_info->manuf_url))) {
		ERR("The length of manufacturer_url(%s) is invalid.", platform_info->manuf_url);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	arg_info = icl_dbus_platform_info_to_gvariant(platform_info);
	ic_dbus_call_register_platform_info_sync(icl_dbus_get_object(), arg_info, &ret,
			NULL, &error);
	if (error) {
		ERR("ic_dbus_call_register_platform_info_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_info);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return ret;
}


static void _icl_platform_info_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	char *uri_path;
	iotcon_platform_info_s *info = calloc(1, sizeof(iotcon_platform_info_s));
	icl_platform_info_s *cb_container = user_data;
	iotcon_platform_info_cb cb = cb_container->cb;

	g_variant_get(parameters, "(&s&s&s&s&s&s&s&s&s&s&s&s)",
			&uri_path,
			&info->platform_id,
			&info->manuf_name,
			&info->manuf_url,
			&info->model_number,
			&info->date_of_manufacture,
			&info->platform_ver,
			&info->os_ver,
			&info->hardware_ver,
			&info->firmware_ver,
			&info->support_url,
			&info->system_time);

	/* From iotivity, we can get uri_path. But, the value is always "/oic/p". */

	if (cb)
		cb(info, cb_container->user_data);
}


API int iotcon_get_platform_info(const char *host_address, iotcon_platform_info_cb cb,
		void *user_data)
{
	int ret;
	GError *error = NULL;
	unsigned int sub_id;
	int signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};
	icl_platform_info_s *cb_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	signal_number = icl_dbus_generate_signal_number();

	ic_dbus_call_get_platform_info_sync(icl_dbus_get_object(), host_address,
			signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_get_platform_info_sync() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_PLATFORM,
			signal_number);

	cb_container = calloc(1, sizeof(icl_platform_info_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, free,
			_icl_platform_info_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}

