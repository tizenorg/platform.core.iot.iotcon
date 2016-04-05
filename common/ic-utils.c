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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <system_info.h>
#include <system_settings.h>

#include "iotcon-types.h"
#include "ic-common.h"
#include "ic-log.h"
#include "ic-utils.h"

#ifdef TZ_VER_3
static int _ic_oic_feature_supported = -1;
#endif

static const char *IC_SYSTEM_INFO_PLATFORM_VERSION = "http://tizen.org/feature/platform.version";
static const char *IC_SYSTEM_INFO_MANUF_NAME = "http://tizen.org/system/manufacturer";
static const char *IC_SYSTEM_INFO_MODEL_NAME = "http://tizen.org/system/model_name";
static const char *IC_SYSTEM_INFO_BUILD_STRING = "http://tizen.org/system/build.string";
static const char *IC_SYSTEM_INFO_TIZEN_ID = "http://tizen.org/system/tizenid";
static const char *IC_PRIV_FILE_NETWORK_GET = "/opt/usr/data/iotcon/iotcon-network-get";
static const char *IC_PRIV_FILE_INTERNET = "/opt/usr/data/iotcon/iotcon-internet";

char* ic_utils_strdup(const char *src)
{
	char *dest = NULL;

	RETV_IF(NULL == src, NULL);

	errno = 0;
	dest = strdup(src);
	if (NULL == dest) {
		ERR("strdup() Fail(%d)", errno);
		return NULL;
	}

	return dest;
}

bool ic_utils_check_oic_feature_supported()
{
#ifdef TZ_VER_3
	if (_ic_oic_feature_supported < 0) {
		bool feature_supported = false;
		system_info_get_platform_bool(IC_FEATURE_OIC, &feature_supported);
		_ic_oic_feature_supported = feature_supported ? 1 : 0;
	}
	return _ic_oic_feature_supported;
#else
	return true;
#endif
}

void ic_utils_free_platform_info(OCPlatformInfo *platform_info)
{
	RET_IF(NULL == platform_info);

	free(platform_info->manufacturerName);
	free(platform_info->manufacturerUrl);
	free(platform_info->modelNumber);
	free(platform_info->dateOfManufacture);
	free(platform_info->platformVersion);
	free(platform_info->operatingSystemVersion);
	free(platform_info->hardwareVersion);
	free(platform_info->firmwareVersion);
	free(platform_info->supportUrl);
	free(platform_info->systemTime);
}

int ic_utils_get_platform_info(OCPlatformInfo *platform_info)
{
	int ret;

	RETV_IF(NULL == platform_info, IOTCON_ERROR_INVALID_PARAMETER);

	/* Mandatory (oic.wk.p) */
	ret = system_info_get_platform_string(IC_SYSTEM_INFO_TIZEN_ID,
			&platform_info->platformID);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string(tizen_id) Fail(%d)", ret);
		ic_utils_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	/* Mandatory (oic.wk.p) */
	ret = system_info_get_platform_string(IC_SYSTEM_INFO_MANUF_NAME,
			&platform_info->manufacturerName);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string(manufacturer) Fail(%d)", ret);
		ic_utils_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = system_info_get_platform_string(IC_SYSTEM_INFO_MODEL_NAME,
			&platform_info->modelNumber);
	if (SYSTEM_INFO_ERROR_NONE != ret)
		WARN("system_info_get_platform_string(model_name) Fail(%d)", ret);

	ret = system_info_get_platform_string(IC_SYSTEM_INFO_PLATFORM_VERSION,
			&platform_info->platformVersion);
	if (SYSTEM_INFO_ERROR_NONE != ret)
		WARN("system_info_get_platform_string(platform_version) Fail(%d)", ret);

	ret = system_info_get_platform_string(IC_SYSTEM_INFO_BUILD_STRING,
			&platform_info->firmwareVersion);
	if (SYSTEM_INFO_ERROR_NONE != ret)
		WARN("system_info_get_platform_string(build_string) Fail(%d)", ret);

	/* platform_info.manufacturerUrl */
	/* platform_info.dateOfManufacture */
	/* platform_info.operatingSystemVersion */
	/* platform_info.hardwareVersion */
	/* platform_info.supportUrl */
	/* platform_info.systemTime */

	return IOTCON_ERROR_NONE;
}

bool ic_utils_check_permission()
{
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

		ret = access(IC_PRIV_FILE_INTERNET, R_OK);
		if (0 != ret) {
			ERR("Permission denied(internet)");
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
}

