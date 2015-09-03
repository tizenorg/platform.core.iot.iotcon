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
#include <iotcon.h>
#include "test.h"

int main()
{
	FN_CALL;
	int ret;
	GMainLoop *loop;
	char *device_name;

	iotcon_platform_info_s *platform_info = calloc(1, sizeof(iotcon_platform_info_s));

	platform_info->platform_id = "platform_id";
	platform_info->manuf_name = "manuf_name";
	platform_info->manuf_url = "manuf_url";
	platform_info->model_number = "model_number";
	platform_info->date_of_manufacture = "date_of_manufacture";
	platform_info->platform_ver = "platform_ver";
	platform_info->os_ver = "os_ver";
	platform_info->hardware_ver = "hardware_ver";
	platform_info->firmware_ver = "firmware_ver";
	platform_info->support_url = "support_url";
	platform_info->system_time = "system_time";

	device_name = "device_name";

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	iotcon_open();

	ret = iotcon_register_platform_info(platform_info);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_register_platform_info() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_register_device_info(device_name);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_register_platform_info() Fail(%d)", ret);
		return -1;
	}

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* iotcon close */
	iotcon_close();

	return 0;
}




