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

#include "ic-utils.h"
#include "icl.h"
#include "icl-dbus.h"

API int iotcon_connect(void)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	ret = icl_dbus_start();
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_start() Fail(%d)", ret);

	return ret;
}


API void iotcon_disconnect(void)
{
	icl_dbus_stop();
}

API int iotcon_get_timeout(int *timeout_seconds)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == timeout_seconds, IOTCON_ERROR_INVALID_PARAMETER);

	*timeout_seconds = icl_dbus_get_timeout();

	return IOTCON_ERROR_NONE;
}


API int iotcon_set_timeout(int timeout_seconds)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	if (ICL_DBUS_TIMEOUT_MAX < timeout_seconds || timeout_seconds <= 0) {
		ERR("timeout_seconds(%d) must be in range from 1 to 3600", timeout_seconds);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = icl_dbus_set_timeout(timeout_seconds);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_set_timeout() Fail(%d)", ret);

	return ret;
}
