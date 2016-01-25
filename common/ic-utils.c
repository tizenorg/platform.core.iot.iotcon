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

#include "iotcon-types.h"
#include "ic-common.h"
#include "ic-log.h"
#include "ic-utils.h"


static int _ic_oic_feature_supported = -1;

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


int ic_utils_convert_interface_flag(iotcon_interface_e src, char **dest)
{
	switch (src) {
	case IOTCON_INTERFACE_DEFAULT:
		*dest = IC_INTERFACE_DEFAULT;
		break;
	case IOTCON_INTERFACE_LINK:
		*dest = IC_INTERFACE_LINK;
		break;
	case IOTCON_INTERFACE_BATCH:
		*dest = IC_INTERFACE_BATCH;
		break;
	case IOTCON_INTERFACE_GROUP:
		*dest = IC_INTERFACE_GROUP;
		break;
	case IOTCON_INTERFACE_READONLY:
		*dest = IC_INTERFACE_READONLY;
		break;
	case IOTCON_INTERFACE_NONE:
	default:
		ERR("Invalid interface(%d)", src);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


int ic_utils_convert_interface_string(const char *src, iotcon_interface_e *dest)
{
	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	if (IC_STR_EQUAL == strcmp(IC_INTERFACE_DEFAULT, src)) {
		*dest = IOTCON_INTERFACE_DEFAULT;
	} else if (IC_STR_EQUAL == strcmp(IC_INTERFACE_LINK, src)) {
		*dest = IOTCON_INTERFACE_LINK;
	} else if (IC_STR_EQUAL == strcmp(IC_INTERFACE_BATCH, src)) {
		*dest = IOTCON_INTERFACE_BATCH;
	} else if (IC_STR_EQUAL == strcmp(IC_INTERFACE_GROUP, src)) {
		*dest = IOTCON_INTERFACE_GROUP;
	} else if (IC_STR_EQUAL == strcmp(IC_INTERFACE_READONLY, src)) {
		*dest = IOTCON_INTERFACE_READONLY;
	} else {
		ERR("Invalid Interface(%s)", src);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
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
		_ic_oic_feature_supported = feature_supported? 1: 0;
	}
	return _ic_oic_feature_supported;
#else
	return true;
#endif
}


