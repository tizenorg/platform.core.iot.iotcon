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

#include "iotcon.h"
#include "ic-common.h"
#include "ic-ioty.h"

/* The length of manufacturer_name should be less than and equal to 16.
 * The length of manufacturer_url should be less than and equal to 32. */
API int iotcon_register_device_info(iotcon_device_info_s device_info)
{
	int ret;

	if (device_info.manuf_name
			&& (IOTCON_MANUFACTURER_NAME_LENGTH_MAX < strlen(device_info.manuf_name))) {
		ERR("The length of manufacturer_name(%s) is invalid.", device_info.manuf_name);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if (device_info.manuf_url
			&& (IOTCON_MANUFACTURER_URL_LENGTH_MAX < strlen(device_info.manuf_url))) {
		ERR("The length of manufacturer_url(%s) is invalid.", device_info.manuf_url);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}


	ret = ic_ioty_register_device_info(device_info);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_register_device_info() Fail(%d)", ret);

	return ret;
}


/* host_address should begin with "coap://" */
API int iotcon_get_device_info(const char *host_address, iotcon_device_info_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret = IOTCON_ERROR_NONE;

	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = ic_ioty_get_device_info(host_address, cb, user_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_get_device_info() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}
