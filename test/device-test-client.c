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

static void _get_platform_info(iotcon_repr_h repr, void *user_data)
{
	FN_CALL;
	char *platform_id = NULL;
	char *manuf_name = NULL;
	char *manuf_url = NULL;
	char *model_number = NULL;
	char *date_of_manufacture = NULL;
	char *platform_ver = NULL;
	char *os_ver = NULL;
	char *hardware_ver = NULL;
	char *firmware_ver = NULL;
	char *support_url = NULL;
	char *system_time = NULL;

	iotcon_repr_get_str(repr, IOTCON_PLATFORM_PLATFORM_ID, &platform_id);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_MFG_NAME, &manuf_name);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_MFG_URL, &manuf_url);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_MODEL_NUM, &model_number);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_MFG_DATE, &date_of_manufacture);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_PLATFORM_VERSION, &platform_ver);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_OS_VERSION, &os_ver);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_HARDWARE_VERSION, &hardware_ver);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_FIRMWARE_VERSION, &firmware_ver);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_SUPPORT_URL, &support_url);
	iotcon_repr_get_str(repr, IOTCON_PLATFORM_SYSTEM_TIME, &system_time);

	INFO("platform_id : %s", platform_id);
	INFO("manuf_name : %s", manuf_name);
	INFO("manuf_url : %s", manuf_url);
	INFO("model_number : %s", model_number);
	INFO("date_of_manufacture : %s", date_of_manufacture);
	INFO("platform_ver : %s", platform_ver);
	INFO("os_ver : %s", os_ver);
	INFO("hardware_ver : %s", hardware_ver);
	INFO("firmware_ver : %s", firmware_ver);
	INFO("support_url : %s", support_url);
	INFO("system_time : %s", system_time);
}

int main()
{
	FN_CALL;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	iotcon_open();

	iotcon_get_platform_info(IOTCON_MULTICAST_ADDRESS, _get_platform_info, NULL);

	g_main_loop_run(loop);

	g_main_loop_unref(loop);

	/* iotcon close */
	iotcon_close();

	return 0;
}
