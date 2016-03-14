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
#include <glib.h>
#include <glib-object.h>

#include "iotcon-internal.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-dbus.h"

#include "icl-ioty.h"

static GThread *icl_thread;
static iotcon_service_mode_e icl_service_mode;
static int icl_timeout_seconds = ICL_DBUS_TIMEOUT_DEFAULT;

iotcon_service_mode_e icl_get_service_mode()
{
	return icl_service_mode;
}

API int iotcon_connect_for_service_mode(iotcon_service_mode_e mode)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(mode < IOTCON_SERVICE_IP || IOTCON_SERVICE_BT < mode,
			IOTCON_ERROR_INVALID_PARAMETER);

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif
	icl_service_mode = mode;

	switch (mode) {
	case IOTCON_SERVICE_IP:
		icl_thread = icl_ioty_init();
		if (NULL == icl_thread) {
			ERR("icl_ioty_init() Fail");
			return -1;
		}

		ret = icl_ioty_set_device_info();
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_set_device_info() Fail(%d)", ret);
			icl_ioty_deinit(icl_thread);
			return -1;
		}

		ret = icl_ioty_set_platform_info();
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_set_platform_info() Fail(%d)", ret);
			icl_ioty_deinit(icl_thread);
			return -1;
		}
		break;
	case IOTCON_SERVICE_BT:
		ret = icl_dbus_start();
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_dbus_start() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid mode (%d)", icl_service_mode);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}

API int iotcon_connect(void)
{
	int ret;

	ret = iotcon_connect_for_service_mode(IOTCON_SERVICE_IP);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_connect_for_service_mode() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}

API void iotcon_disconnect(void)
{
	switch (icl_service_mode) {
	case IOTCON_SERVICE_IP:
		icl_ioty_deinit(icl_thread);
		icl_thread = 0;
		break;
	case IOTCON_SERVICE_BT:
		icl_dbus_stop();
		break;
	default:
		ERR("Invalid serveice (%d)", icl_service_mode);
		return;
	}
}

API int iotcon_get_timeout(int *timeout_seconds)
{
	iotcon_service_mode_e mode;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == timeout_seconds, IOTCON_ERROR_INVALID_PARAMETER);

	mode = icl_get_service_mode();

	switch (mode) {
	case IOTCON_SERVICE_IP:
		*timeout_seconds = icl_timeout_seconds;
		break;
	case IOTCON_SERVICE_BT:
		*timeout_seconds = icl_dbus_get_timeout();
		break;
	default:
		ERR("Invalid mode(%d)", mode);
		return IOTCON_ERROR_SYSTEM;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_set_timeout(int timeout_seconds)
{
	int ret;
	iotcon_service_mode_e mode;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	if (ICL_DBUS_TIMEOUT_MAX < timeout_seconds || timeout_seconds <= 0) {
		ERR("timeout_seconds(%d) must be in range from 1 to 3600", timeout_seconds);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	mode = icl_get_service_mode();

	switch (mode) {
	case IOTCON_SERVICE_IP:
		icl_timeout_seconds = timeout_seconds;
		break;
	case IOTCON_SERVICE_BT:
		ret = icl_dbus_set_timeout(timeout_seconds);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_dbus_set_timeout() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid mode(%d)", mode);
		return IOTCON_ERROR_SYSTEM;
	}

	return IOTCON_ERROR_NONE;
}
