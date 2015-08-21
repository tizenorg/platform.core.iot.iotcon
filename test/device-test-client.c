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

static void _get_platform_info(iotcon_platform_info_s info, void *user_data)
{
	INFO("platform_id : %s", info.platform_id);
	INFO("manuf_name : %s", info.manuf_name);
	INFO("manuf_url : %s", info.manuf_url);
	INFO("model_number : %s", info.model_number);
	INFO("date_of_manufacture : %s", info.date_of_manufacture);
	INFO("platform_ver : %s", info.platform_ver);
	INFO("os_ver : %s", info.os_ver);
	INFO("hardware_ver : %s", info.hardware_ver);
	INFO("firmware_ver : %s", info.firmware_ver);
	INFO("support_url : %s", info.support_url);
	INFO("system_time : %s", info.system_time);
}


static void _get_device_info(const char *device_name, const char *sid,
		const char *spec_version, const char *data_model_version, void *user_data)
{
	INFO("device_name : %s", device_name);
	INFO("sid : %s", sid);
	INFO("spec_version : %s", spec_version);
	INFO("data_model_version : %s", data_model_version);
}


int main()
{
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	iotcon_open();

	iotcon_get_platform_info(IOTCON_MULTICAST_ADDRESS, _get_platform_info, NULL);

	iotcon_get_device_info(IOTCON_MULTICAST_ADDRESS, _get_device_info, NULL);

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* iotcon close */
	iotcon_close();

	return 0;
}
