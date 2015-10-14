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
#include <errno.h>
#include <glib.h>
#include <iotcon.h>
#include "test.h"

static int _set_device_info()
{
	int ret;
	char *device_name = "device name";
	iotcon_device_info_h device_info;

	ret = iotcon_device_info_create(&device_info);
	if (NULL == device_info) {
		ERR("iotcon_device_info_create() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_device_info_set_property(device_info, IOTCON_DEVICE_INFO_NAME,
			device_name);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_device_info_set_property() Fail(%d)", ret);
		iotcon_device_info_destroy(device_info);
		return -1;
	}

	ret = iotcon_set_device_info(device_info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_set_device_info() Fail(%d)", ret);
		iotcon_device_info_destroy(device_info);
		return -1;
	}

	iotcon_device_info_destroy(device_info);

	return 0;
}

static int _set_platform_info()
{
	int ret;
	char *platform_id = "platform_id";
	char *manuf_name = "manuf_name";
	char *manuf_url = "manuf_url";
	char *model_number = "model_number";
	char *date_of_manuf = "date_of_manuf";
	char *platform_ver = "platform_ver";
	char *os_ver = "os_ver";
	char *hardware_ver = "hardware_ver";
	char *firmware_ver = "firmware_ver";
	char *support_url = "support_url";
	char *system_time = "system_time";
	iotcon_platform_info_h platform_info;

	ret = iotcon_platform_info_create(&platform_info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_create() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_platform_info_set_property(platform_info, IOTCON_PLATFORM_INFO_ID,
			platform_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_MANUF_NAME, manuf_name);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info, IOTCON_PLATFORM_INFO_MANUF_URL,
			manuf_url);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_MODEL_NUMBER, model_number);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_DATE_OF_MANUF, date_of_manuf);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_PLATFORM_VER, platform_ver);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info, IOTCON_PLATFORM_INFO_OS_VER,
			os_ver);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_HARDWARE_VER, hardware_ver);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_FIRMWARE_VER, firmware_ver);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_SUPPORT_URL, support_url);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}
	ret = iotcon_platform_info_set_property(platform_info,
			IOTCON_PLATFORM_INFO_SYSTEM_TIME, system_time);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_platform_info_set_property() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}

	ret = iotcon_set_platform_info(platform_info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_set_platform_info() Fail(%d)", ret);
		iotcon_platform_info_destroy(platform_info);
		return -1;
	}

	return 0;
}

int main()
{
	FN_CALL;
	int ret;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	ret = iotcon_open();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_open() Fail(%d)", ret);
		return -1;
	}

	ret = _set_device_info();
	if (0 != ret) {
		ERR("_set_device_info() Fail");
		iotcon_close();
		return -1;
	}
	ret = _set_platform_info();
	if (0 != ret) {
		ERR("_set_platform_info() Fail");
		iotcon_close();
		return -1;
	}

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* iotcon close */
	iotcon_close();

	return 0;
}

