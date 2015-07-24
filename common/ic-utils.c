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
#include <string.h>
#include <errno.h>

#include "iotcon-constant.h"
#include "ic-common.h"
#include "ic-log.h"
#include "ic-utils.h"

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
	case IOTCON_INTERFACE_NONE:
	default:
		ERR("Invalid interface(%d)", src);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}



