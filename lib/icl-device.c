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


API int iotcon_device_info_create(iotcon_device_info_h *device_info)
{
	iotcon_device_info_h info = NULL;

	RETV_IF(NULL == device_info, IOTCON_ERROR_INVALID_PARAMETER);

	info = calloc(1, sizeof(struct icl_device_info));
	if (NULL == info) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	*device_info = info;

	return IOTCON_ERROR_NONE;
}


API void iotcon_device_info_destroy(iotcon_device_info_h device_info)
{
	RET_IF(NULL == device_info);

	free(device_info->device_name);
	free(device_info->spec_ver);
	free(device_info->device_id);
	free(device_info->data_model_ver);
	free(device_info);
}


API int iotcon_device_info_set_property(iotcon_device_info_h device_info,
		iotcon_device_info_e property, const char *value)
{
	char *dup_value;

	RETV_IF(NULL == device_info, IOTCON_ERROR_INVALID_PARAMETER);

	if (value) {
		dup_value = strdup(value);
		if (NULL == dup_value) {
			ERR("strdup() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
	} else {
		dup_value = NULL;
	}

	switch (property) {
	case IOTCON_DEVICE_INFO_NAME:
		if (device_info->device_name)
			free(device_info->device_name);
		device_info->device_name = dup_value;
		break;
	case IOTCON_DEVICE_INFO_SPEC_VER:
		if (device_info->spec_ver)
			free(device_info->spec_ver);
		device_info->spec_ver = dup_value;
		break;
	case IOTCON_DEVICE_INFO_ID:
		if (device_info->device_id)
			free(device_info->device_id);
		device_info->device_id = dup_value;
		break;
	case IOTCON_DEVICE_INFO_DATA_MODEL_VER:
		if (device_info->data_model_ver)
			free(device_info->data_model_ver);
		device_info->data_model_ver = dup_value;
		break;
	default:
		ERR("Invalid property(%d)", property);
		free(dup_value);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_device_info_get_property(iotcon_device_info_h device_info,
		iotcon_device_info_e property, char **value)
{
	RETV_IF(NULL == device_info, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);

	switch (property) {
	case IOTCON_DEVICE_INFO_NAME:
		*value = device_info->device_name;
		break;
	case IOTCON_DEVICE_INFO_SPEC_VER:
		*value = device_info->spec_ver;
		break;
	case IOTCON_DEVICE_INFO_ID:
		*value = device_info->device_id;
		break;
	case IOTCON_DEVICE_INFO_DATA_MODEL_VER:
		*value = device_info->data_model_ver;
		break;
	default:
		ERR("Invalid property(%d)", property);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_set_device_info(iotcon_device_info_h device_info)
{
	int ret;
	GError *error = NULL;
	GVariant *arg_info;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == device_info, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device_info->device_name, IOTCON_ERROR_INVALID_PARAMETER);

	arg_info = icl_dbus_device_info_to_gvariant(device_info);
	ic_dbus_call_register_device_info_sync(icl_dbus_get_object(), arg_info, &ret,
			NULL, &error);
	if (error) {
		ERR("ic_dbus_call_register_device_info_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		g_variant_unref(arg_info);
		return ret;
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
	char *uri_path;
	struct icl_device_info info = {0};
	icl_device_info_s *cb_container = user_data;
	iotcon_device_info_cb cb = cb_container->cb;

	g_variant_get(parameters, "(&s&s&s&s&s)", &uri_path, &info.device_name,
			&info.spec_ver, &info.device_id, &info.data_model_ver);

	/* From iotivity, we can get uri_path. But, the value is always "/oic/d". */

	if (cb)
		cb(&info, cb_container->user_data);
}


API int iotcon_get_device_info(const char *host_address, iotcon_device_info_cb cb,
		void *user_data)
{
	GError *error = NULL;
	unsigned int sub_id;
	int ret, signal_number;
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
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
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
		free(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}


API int iotcon_platform_info_create(iotcon_platform_info_h *platform_info)
{
	iotcon_platform_info_h info = NULL;

	RETV_IF(NULL == platform_info, IOTCON_ERROR_INVALID_PARAMETER);

	info = calloc(1, sizeof(struct icl_platform_info));
	if (NULL == info) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	*platform_info = info;

	return IOTCON_ERROR_NONE;
}


API void iotcon_platform_info_destroy(iotcon_platform_info_h platform_info)
{
	RET_IF(NULL == platform_info);

	free(platform_info->platform_id);
	free(platform_info->manuf_name);
	free(platform_info->manuf_url);
	free(platform_info->model_number);
	free(platform_info->date_of_manuf);
	free(platform_info->platform_ver);
	free(platform_info->os_ver);
	free(platform_info->hardware_ver);
	free(platform_info->firmware_ver);
	free(platform_info->support_url);
	free(platform_info->system_time);
	free(platform_info);
}


API int iotcon_platform_info_set_property(iotcon_platform_info_h platform_info,
		iotcon_platform_info_e property, const char *value)
{
	char *dup_value;

	RETV_IF(NULL == platform_info, IOTCON_ERROR_INVALID_PARAMETER);

	if (value) {
		dup_value = strdup(value);
		if (NULL == dup_value) {
			ERR("strdup() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
	} else {
		dup_value = NULL;
	}

	switch (property) {
	case IOTCON_PLATFORM_INFO_ID:
		if (platform_info->platform_id)
			free(platform_info->platform_id);
		platform_info->platform_id = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_MANUF_NAME:
		if (platform_info->manuf_name)
			free(platform_info->manuf_name);
		platform_info->manuf_name = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_MANUF_URL:
		if (platform_info->manuf_url)
			free(platform_info->manuf_url);
		platform_info->manuf_url = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_MODEL_NUMBER:
		if (platform_info->model_number)
			free(platform_info->model_number);
		platform_info->model_number = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_DATE_OF_MANUF:
		if (platform_info->date_of_manuf)
			free(platform_info->date_of_manuf);
		platform_info->date_of_manuf = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_PLATFORM_VER:
		if (platform_info->platform_ver)
			free(platform_info->platform_ver);
		platform_info->platform_ver = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_OS_VER:
		if (platform_info->os_ver)
			free(platform_info->os_ver);
		platform_info->os_ver = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_HARDWARE_VER:
		if (platform_info->hardware_ver)
			free(platform_info->hardware_ver);
		platform_info->hardware_ver = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_FIRMWARE_VER:
		if (platform_info->firmware_ver)
			free(platform_info->firmware_ver);
		platform_info->firmware_ver = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_SUPPORT_URL:
		if (platform_info->support_url)
			free(platform_info->support_url);
		platform_info->support_url = dup_value;
		break;
	case IOTCON_PLATFORM_INFO_SYSTEM_TIME:
		if (platform_info->system_time)
			free(platform_info->system_time);
		platform_info->system_time = dup_value;
		break;
	default:
		ERR("Invalid property(%d)", property);
		free(dup_value);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_platform_info_get_property(iotcon_platform_info_h platform_info,
		iotcon_platform_info_e property, char **value)
{
	RETV_IF(NULL == platform_info, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);

	switch (property) {
	case IOTCON_PLATFORM_INFO_ID:
		*value = platform_info->platform_id;
		break;
	case IOTCON_PLATFORM_INFO_MANUF_NAME:
		*value = platform_info->manuf_name;
		break;
	case IOTCON_PLATFORM_INFO_MANUF_URL:
		*value = platform_info->manuf_url;
		break;
	case IOTCON_PLATFORM_INFO_MODEL_NUMBER:
		*value = platform_info->model_number;
		break;
	case IOTCON_PLATFORM_INFO_DATE_OF_MANUF:
		*value = platform_info->date_of_manuf;
		break;
	case IOTCON_PLATFORM_INFO_PLATFORM_VER:
		*value = platform_info->platform_ver;
		break;
	case IOTCON_PLATFORM_INFO_OS_VER:
		*value = platform_info->os_ver;
		break;
	case IOTCON_PLATFORM_INFO_HARDWARE_VER:
		*value = platform_info->hardware_ver;
		break;
	case IOTCON_PLATFORM_INFO_FIRMWARE_VER:
		*value = platform_info->firmware_ver;
		break;
	case IOTCON_PLATFORM_INFO_SUPPORT_URL:
		*value = platform_info->support_url;
		break;
	case IOTCON_PLATFORM_INFO_SYSTEM_TIME:
		*value = platform_info->system_time;
		break;
	default:
		ERR("Invalid property(%d)", property);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


/* The length of manufacturer_name should be less than and equal to 16.
 * The length of manufacturer_url should be less than and equal to 32. */
API int iotcon_set_platform_info(iotcon_platform_info_h platform_info)
{
	int ret;
	GError *error = NULL;
	GVariant *arg_info;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == platform_info, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == platform_info->platform_id
			|| NULL == platform_info->manuf_name
			|| NULL == platform_info->manuf_url
			|| NULL == platform_info->model_number
			|| NULL == platform_info->date_of_manuf
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
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		g_variant_unref(arg_info);
		return ret;
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
	struct icl_platform_info info = {0};
	icl_platform_info_s *cb_container = user_data;
	iotcon_platform_info_cb cb = cb_container->cb;

	g_variant_get(parameters, "(&s&s&s&s&s&s&s&s&s&s&s&s)",
			&uri_path,
			&info.platform_id,
			&info.manuf_name,
			&info.manuf_url,
			&info.model_number,
			&info.date_of_manuf,
			&info.platform_ver,
			&info.os_ver,
			&info.hardware_ver,
			&info.firmware_ver,
			&info.support_url,
			&info.system_time);

	/* From iotivity, we can get uri_path. But, the value is always "/oic/p". */

	if (cb)
		cb(&info, cb_container->user_data);
}


API int iotcon_get_platform_info(const char *host_address, iotcon_platform_info_cb cb,
		void *user_data)
{
	GError *error = NULL;
	unsigned int sub_id;
	int ret, signal_number;
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
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
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
		free(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}
