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
#include <inttypes.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "iotcon-internal.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-representation.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-device.h"
#include "icl-ioty.h"

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
	bool found;
	iotcon_device_info_cb cb;
	void *user_data;
	unsigned int id;
	int timeout_id;
} icl_device_info_s;

typedef struct {
	bool found;
	iotcon_platform_info_cb cb;
	void *user_data;
	unsigned int id;
	int timeout_id;
} icl_platform_info_s;


API int iotcon_device_info_get_property(iotcon_device_info_h device_info,
		iotcon_device_info_e property, char **value)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
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
	struct icl_device_info info = {0};
	icl_device_info_s *cb_container = user_data;
	iotcon_device_info_cb cb = cb_container->cb;

	cb_container->found = true;

	g_variant_get(parameters, "(&s&s&s&s)", &info.device_name,
			&info.spec_ver, &info.device_id, &info.data_model_ver);

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

	if (false == cb_container->found && cb_container->cb)
		cb_container->cb(&info, IOTCON_ERROR_TIMEOUT, cb_container->user_data);
	cb_container->timeout_id = 0;

	icl_dbus_unsubscribe_signal(cb_container->id);
	cb_container->id = 0;

	return G_SOURCE_REMOVE;
}

static void _icl_device_info_conn_cleanup(icl_device_info_s *cb_container)
{
	RET_IF(NULL == cb_container);
	if (cb_container->timeout_id)
		g_source_remove(cb_container->timeout_id);
	free(cb_container);
}

static int _icl_get_device_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_device_info_cb cb,
		void *user_data)
{
	int ret, timeout;
	unsigned int sub_id;
	GError *error = NULL;
	icl_device_info_s *cb_container;
	int64_t signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);

	timeout = icl_dbus_get_timeout();

	ic_dbus_call_get_device_info_sync(icl_dbus_get_object(),
			ic_utils_dbus_encode_str(host_address),
			connectivity_type,
			timeout,
			&signal_number,
			&ret,
			NULL,
			&error);
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

	snprintf(signal_name, sizeof(signal_name), "%s_%"PRIx64, IC_DBUS_SIGNAL_DEVICE,
			signal_number);

	cb_container = calloc(1, sizeof(icl_device_info_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container,
			_icl_device_info_conn_cleanup, _icl_device_info_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		free(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	cb_container->timeout_id = g_timeout_add_seconds(timeout,
			_icl_timeout_get_device_info, cb_container);

	return IOTCON_ERROR_NONE;
}

API int iotcon_get_device_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_device_info_cb cb,
		void *user_data)
{
	int ret, conn_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_check_connectivity_type(connectivity_type, icl_get_service_mode());
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_check_connectivity_type() Fail(%d)", ret);
		return ret;
	}

	conn_type = connectivity_type;

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_get_device_info(host_address, connectivity_type, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_get_device_info() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		ret = _icl_get_device_info(host_address, connectivity_type, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_get_device_info() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", conn_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_platform_info_get_property(iotcon_platform_info_h platform_info,
		iotcon_platform_info_e property, char **value)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
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
	struct icl_platform_info info = {0};
	icl_platform_info_s *cb_container = user_data;
	iotcon_platform_info_cb cb = cb_container->cb;

	cb_container->found = true;

	g_variant_get(parameters, "(&s&s&s&s&s&s&s&s&s&s&s)",
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

	info.manuf_url = ic_utils_dbus_decode_str(info.manuf_url);
	info.model_number = ic_utils_dbus_decode_str(info.model_number);
	info.date_of_manuf = ic_utils_dbus_decode_str(info.date_of_manuf);
	info.platform_ver = ic_utils_dbus_decode_str(info.platform_ver);
	info.os_ver = ic_utils_dbus_decode_str(info.os_ver);
	info.hardware_ver = ic_utils_dbus_decode_str(info.hardware_ver);
	info.firmware_ver = ic_utils_dbus_decode_str(info.firmware_ver);
	info.support_url = ic_utils_dbus_decode_str(info.support_url);
	info.system_time = ic_utils_dbus_decode_str(info.system_time);

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

	if (false == cb_container->found && cb_container->cb)
		cb_container->cb(&info, IOTCON_ERROR_TIMEOUT, cb_container->user_data);
	cb_container->timeout_id = 0;

	icl_dbus_unsubscribe_signal(cb_container->id);
	cb_container->id = 0;

	return G_SOURCE_REMOVE;
}

static void _icl_platform_info_conn_cleanup(icl_platform_info_s *cb_container)
{
	RET_IF(NULL == cb_container);

	if (cb_container->timeout_id)
		g_source_remove(cb_container->timeout_id);

	free(cb_container);
}

static int _icl_get_platform_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_platform_info_cb cb,
		void *user_data)
{
	int ret, timeout;
	unsigned int sub_id;
	GError *error = NULL;
	icl_platform_info_s *cb_container;
	int64_t signal_number;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);

	timeout = icl_dbus_get_timeout();

	ic_dbus_call_get_platform_info_sync(icl_dbus_get_object(),
			ic_utils_dbus_encode_str(host_address),
			connectivity_type,
			timeout,
			&signal_number,
			&ret,
			NULL,
			&error);
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

	snprintf(signal_name, sizeof(signal_name), "%s_%"PRIx64, IC_DBUS_SIGNAL_PLATFORM,
			signal_number);

	cb_container = calloc(1, sizeof(icl_platform_info_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container,
			_icl_platform_info_conn_cleanup, _icl_platform_info_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		free(cb_container);
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;
	cb_container->timeout_id = g_timeout_add_seconds(timeout,
			_icl_timeout_get_platform_info, cb_container);

	return IOTCON_ERROR_NONE;
}

API int iotcon_get_platform_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_platform_info_cb cb,
		void *user_data)
{
	int ret, conn_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_check_connectivity_type(connectivity_type, icl_get_service_mode());
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_check_connectivity_type() Fail(%d)", ret);
		return ret;
	}

	conn_type = connectivity_type;

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_get_platform_info(host_address, connectivity_type, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_get_platform_info() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		ret = _icl_get_platform_info(host_address, connectivity_type, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_get_platform_info() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", conn_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}
