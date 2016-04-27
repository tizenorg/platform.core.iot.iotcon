/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include "icl-ioty.h"

#define ICL_TIMEOUT_DEFAULT 30 /* 30 sec */
#define ICL_TIMEOUT_MAX 60*60 /* 60 min */

static GThread *icl_thread;
static int icl_timeout_seconds = ICL_TIMEOUT_DEFAULT;
static int icl_connection_count;

/*
#ifdef TZ_VER_3
static const char *IC_PRIV_FILE_NETWORK_GET = "/usr/share/iotcon/iotcon-network-get";
#endif
*/

static bool _ic_check_permission()
{
	return true;
// TODO: Can't access file in user side daemon
/*
#ifdef TZ_VER_3
	int ret;
	static int have_permission = -1;

	if (-1 == have_permission) {
		ret = access(IC_PRIV_FILE_NETWORK_GET, R_OK);
		if (0 != ret) {
			ERR("Permission denied(network.get)");
			have_permission = 0;
			return false;
		}
		have_permission = 1;
	} else if (0 == have_permission) {
		return false;
	}
	return true;
#else
	return true;
#endif
*/
}

API int iotcon_initialize()
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(false == _ic_check_permission(), IOTCON_ERROR_PERMISSION_DENIED);

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif
	icl_connection_count++;
	if (1 == icl_connection_count) {
		ret = icl_ioty_init(&icl_thread);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_init() Fail(%d)", ret);
			return ret;
		}

		ret = icl_ioty_set_platform_info();
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_set_platform_info() Fail(%d)", ret);
			icl_ioty_deinit(icl_thread);
			return ret;
		}
	}

	return IOTCON_ERROR_NONE;
}

API void iotcon_deinitialize(void)
{
	icl_connection_count--;
	if (0 == icl_connection_count) {
		icl_ioty_deinit(icl_thread);
		icl_thread = 0;
	}
}

API int iotcon_get_timeout(int *timeout_seconds)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == timeout_seconds, IOTCON_ERROR_INVALID_PARAMETER);

	*timeout_seconds = icl_timeout_seconds;

	return IOTCON_ERROR_NONE;
}


API int iotcon_set_timeout(int timeout_seconds)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	if (ICL_TIMEOUT_MAX < timeout_seconds || timeout_seconds <= 0) {
		ERR("timeout_seconds(%d) must be in range from 1 to 3600", timeout_seconds);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	icl_timeout_seconds = timeout_seconds;

	return IOTCON_ERROR_NONE;
}


