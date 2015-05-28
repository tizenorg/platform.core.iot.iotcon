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
#include "ic-device.h"

/* The length of manufacturer_name should be less than and equal to 16.
 * The length of manufacturer_url should be less than and equal to 32. */
API int iotcon_register_device_info(
		char *device_name,
		char *host_name,
		char *device_uuid,
		char *content_type,
		char *version,
		char *manufacturer_name,
		char *manufacturer_url,
		char *model_number,
		char *date_of_manufacture,
		char *platform_version,
		char *firmware_version,
		char *support_url)
{
	int ret;
	struct ic_device_info device_info = {0};

	if (manufacturer_name
			&& (IOTCON_MANUFACTURER_NAME_LENGTH_MAX < strlen(manufacturer_name))) {
		ERR("The length of manufacturer_name(%s) is invalid.", manufacturer_name);
		return IOTCON_ERROR_PARAM;
	}

	if (manufacturer_url
			&& (IOTCON_MANUFACTURER_URL_LENGTH_MAX < strlen(manufacturer_url))) {
		ERR("The length of manufacturer_url(%s) is invalid.", manufacturer_url);
		return IOTCON_ERROR_PARAM;
	}

	device_info.device_name = device_name;
	device_info.host_name = host_name;
	device_info.device_uuid = device_uuid;
	device_info.content_type = content_type;
	device_info.version = version;
	device_info.manufacturer_name = manufacturer_name;
	device_info.manufacturer_url = manufacturer_url;
	device_info.model_number = model_number;
	device_info.date_of_manufacture = date_of_manufacture;
	device_info.platform_ver = platform_version;
	device_info.firmware_ver = firmware_version;
	device_info.support_url = support_url;

	ret = ic_ioty_register_device_info(&device_info);
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

	RETV_IF(NULL == host_address, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == cb, IOTCON_ERROR_PARAM);

	ret = ic_ioty_get_device_info(host_address, cb, user_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_get_device_info() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API const char* iotcon_device_info_get_device_name(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->device_name;
}


API const char* iotcon_device_info_get_host_name(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->host_name;
}


API const char* iotcon_device_info_get_device_uuid(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->device_uuid;
}


API const char* iotcon_device_info_get_content_type(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->content_type;
}


API const char* iotcon_device_info_get_version(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->version;
}


API const char* iotcon_device_info_get_manufacturer_name(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->manufacturer_name;
}


API const char* iotcon_device_info_get_manufacturer_url(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->manufacturer_url;
}


API const char* iotcon_device_info_get_model_number(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->model_number;
}


API const char* iotcon_device_info_get_date_of_manufacture(
		iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->date_of_manufacture;
}


API const char* iotcon_device_info_get_platform_version(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->platform_ver;
}


API const char* iotcon_device_info_get_firmware_version(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->firmware_ver;
}


API const char* iotcon_device_info_get_support_url(iotcon_device_info_h device_info)
{
	RETV_IF(NULL == device_info, NULL);

	return device_info->support_url;
}
