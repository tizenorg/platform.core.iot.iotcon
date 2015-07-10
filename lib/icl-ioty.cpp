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
#include <OCApi.h>

extern "C" {
#include "iotcon-constant.h"
#include "ic-utils.h"
#include "icl.h"
}

using namespace std;
using namespace OC;

static int _icl_ioty_convert_interface_flag(iotcon_interface_e src, string &dest)
{
	switch (src) {
	case IOTCON_INTERFACE_GROUP:
		dest = GROUP_INTERFACE;
		break;
	case IOTCON_INTERFACE_BATCH:
		dest = BATCH_INTERFACE;
		break;
	case IOTCON_INTERFACE_LINK:
		dest = LINK_INTERFACE;
		break;
	case IOTCON_INTERFACE_DEFAULT:
		dest = DEFAULT_INTERFACE;
		break;
	case IOTCON_INTERFACE_NONE:
	default:
		ERR("Invalid interface");
		dest = "";
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


extern "C" int icl_ioty_convert_interface_string(const char *src, iotcon_interface_e *dest)
{
	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	if (IC_STR_EQUAL == strcmp(DEFAULT_INTERFACE.c_str(), src)) {
		*dest = IOTCON_INTERFACE_DEFAULT;
	} else if (IC_STR_EQUAL == strcmp(LINK_INTERFACE.c_str(), src)) {
		*dest = IOTCON_INTERFACE_LINK;
	} else if (IC_STR_EQUAL == strcmp(BATCH_INTERFACE.c_str(), src)) {
		*dest = IOTCON_INTERFACE_BATCH;
	} else if (IC_STR_EQUAL == strcmp(GROUP_INTERFACE.c_str(), src)) {
		*dest = IOTCON_INTERFACE_GROUP;
	} else {
		ERR("Invalid interface");
		*dest = IOTCON_INTERFACE_NONE;
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int icl_ioty_convert_interface_flag(iotcon_interface_e src, char **dest)
{
	FN_CALL;
	int ret;
	string iface_str;

	ret = _icl_ioty_convert_interface_flag(src, iface_str);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_ioty_convert_interface_flag() Fail(%d)", ret);
		*dest = NULL;
		return ret;
	}

	*dest = ic_utils_strdup(iface_str.c_str());
	if (NULL == *dest) {
		ERR("ic_utils_strdup() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

