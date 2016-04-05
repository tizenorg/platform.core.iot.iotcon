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


const char* ic_utils_dbus_encode_str(const char *src)
{
	return (src) ? src : IC_STR_NULL;
}


char* ic_utils_dbus_decode_str(char *src)
{
	RETV_IF(NULL == src, NULL);

	if (IC_STR_EQUAL == strcmp(IC_STR_NULL, src))
		return NULL;
	else
		return src;
}


void ic_utils_gvariant_array_free(GVariant **value)
{
	int i;

	for (i = 0; value[i]; i++)
		g_variant_unref(value[i]);

	free(value);
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

int ic_utils_get_device_info(OCDeviceInfo *device_info)
{
	int ret;

	RETV_IF(NULL == device_info, IOTCON_ERROR_INVALID_PARAMETER);

	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_DEVICE_NAME, &device_info->deviceName);
	if (SYSTEM_SETTINGS_ERROR_NONE != ret) {
		ERR("system_settings_get_value_string() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}

	return IOTCON_ERROR_NONE;
}

void ic_utils_free_platform_info(OCPlatformInfo *platform_info)
{
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
		ERR("system_info_get_platform_string() Fail(%d)", ret);
		ic_utils_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	/* Mandatory (oic.wk.p) */
	ret = system_info_get_platform_string(IC_SYSTEM_INFO_MANUF_NAME,
			&platform_info->manufacturerName);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string() Fail(%d)", ret);
		ic_utils_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = system_info_get_platform_string(IC_SYSTEM_INFO_MODEL_NAME,
			&platform_info->modelNumber);
	WARN_IF(SYSTEM_INFO_ERROR_NONE != ret, "system_info_get_platform_string(%s) Fail(%d)",
			IC_SYSTEM_INFO_MODEL_NAME, ret);

	ret = system_info_get_platform_string(IC_SYSTEM_INFO_PLATFORM_VERSION,
			&platform_info->platformVersion);
	WARN_IF(SYSTEM_INFO_ERROR_NONE != ret, "system_info_get_platform_string(%s) Fail(%d)",
			IC_SYSTEM_INFO_PLATFORM_VERSION, ret);

	ret = system_info_get_platform_string(IC_SYSTEM_INFO_BUILD_STRING,
			&platform_info->firmwareVersion);
	WARN_IF(SYSTEM_INFO_ERROR_NONE != ret, "system_info_get_platform_string(%s) Fail(%d)",
			IC_SYSTEM_INFO_BUILD_STRING, ret);

	/* platform_info.manufacturerUrl */
	/* platform_info.dateOfManufacture */
	/* platform_info.operatingSystemVersion */
	/* platform_info.hardwareVersion */
	/* platform_info.supportUrl */
	/* platform_info.systemTime */

	return IOTCON_ERROR_NONE;
}


