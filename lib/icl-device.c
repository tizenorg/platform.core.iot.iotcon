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
#include "icl-representation.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-device.h"

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
	int timeout_id;
} icl_device_info_s;

typedef struct {
	iotcon_platform_info_cb cb;
	void *user_data;
	unsigned int id;
	int timeout_id;
} icl_platform_info_s;

typedef struct {
	iotcon_tizen_info_cb cb;
	void *user_data;
} icl_tizen_info_s;


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

	if (cb_container->timeout_id) {
		g_source_remove(cb_container->timeout_id);
		cb_container->timeout_id = 0;
	}

	g_variant_get(parameters, "(&s&s&s&s&s)", &uri_path, &info.device_name,
			&info.spec_ver, &info.device_id, &info.data_model_ver);

	/* From iotivity, we can get uri_path. But, the value is always "/oic/d". */

	if (cb)
		cb(&info, IOTCON_ERROR_NONE, cb_container->user_data);
}

static gboolean _icl_timeout_get_device_info(gpointer p)
{
	FN_CALL;
	icl_device_info_s *cb_container = p;
	struct icl_device_info info = {0};

	if (NULL == cb_container) {
		ERR("cb_container is NULL");
		return G_SOURCE_REMOVE;
	}

	if (cb_container->cb)
		cb_container->cb(&info, IOTCON_ERROR_TIMEOUT, cb_container->user_data);

	icl_dbus_unsubscribe_signal(cb_container->id);

	return G_SOURCE_REMOVE;
}

API int iotcon_get_device_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_device_info_cb cb,
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
			connectivity_type, signal_number, &ret, NULL, &error);
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

	cb_container->timeout_id = g_timeout_add_seconds(icl_dbus_get_timeout(),
			_icl_timeout_get_device_info, cb_container);

	return ret;
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

	if (cb_container->timeout_id) {
		g_source_remove(cb_container->timeout_id);
		cb_container->timeout_id = 0;
	}

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
		cb(&info, IOTCON_ERROR_NONE, cb_container->user_data);
}

static gboolean _icl_timeout_get_platform_info(gpointer p)
{
	FN_CALL;
	icl_platform_info_s *cb_container = p;
	struct icl_platform_info info = {0};

	if (NULL == cb_container) {
		ERR("cb_container is NULL");
		return G_SOURCE_REMOVE;
	}

	if (cb_container->cb)
		cb_container->cb(&info, IOTCON_ERROR_TIMEOUT, cb_container->user_data);

	icl_dbus_unsubscribe_signal(cb_container->id);

	return G_SOURCE_REMOVE;
}

API int iotcon_get_platform_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_platform_info_cb cb,
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
			connectivity_type, signal_number, &ret, NULL, &error);
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
	cb_container->timeout_id = g_timeout_add_seconds(icl_dbus_get_timeout(),
			_icl_timeout_get_platform_info, cb_container);

	return ret;
}


static void _icl_tizen_info_cb(GObject *object, GAsyncResult *g_async_res,
		gpointer user_data)
{
	int res;
	GVariant *result;
	char *device_name;
	char *tizen_device_id;
	GError *error = NULL;
	struct icl_tizen_info info = {0};
	icl_tizen_info_s *cb_container = user_data;
	iotcon_tizen_info_cb cb = cb_container->cb;

	ic_dbus_call_get_tizen_info_finish(IC_DBUS(object), &result, g_async_res, &error);
	if (error) {
		ERR("ic_dbus_call_get_tizen_info_finish() Fail(%s)", error->message);
		if (cb) {
			int ret = icl_dbus_convert_dbus_error(error->code);
			cb(&info, ret, cb_container->user_data);
		}
		g_error_free(error);
		/* TODO contain time out error */
		free(cb_container);
		return;
	}

	g_variant_get(result, "(&s&si)", &device_name, &tizen_device_id, &res);

	if (IOTCON_ERROR_NONE == res && NULL != ic_utils_dbus_decode_str(tizen_device_id)) {
		info.device_name = ic_utils_dbus_decode_str(device_name);
		info.tizen_device_id = tizen_device_id;
	}

	if (cb)
		cb(&info, res, cb_container->user_data);

	free(cb_container);
}


API int iotcon_get_tizen_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_tizen_info_cb cb,
		void *user_data)
{
	icl_tizen_info_s *cb_container;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = calloc(1, sizeof(icl_tizen_info_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	ic_dbus_call_get_tizen_info(icl_dbus_get_object(), host_address, connectivity_type,
			NULL, _icl_tizen_info_cb, cb_container);

	return IOTCON_ERROR_NONE;
}


API int iotcon_tizen_info_get_property(iotcon_tizen_info_h tizen_info,
		iotcon_tizen_info_e property, char **value)
{
	RETV_IF(NULL == tizen_info, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);

	switch (property) {
	case IOTCON_TIZEN_INFO_DEVICE_NAME:
		*value = tizen_info->device_name;
		break;
	case IOTCON_TIZEN_INFO_TIZEN_DEVICE_ID:
		*value = tizen_info->tizen_device_id;
		break;
	default:
		ERR("Invalid property(%d)", property);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}
