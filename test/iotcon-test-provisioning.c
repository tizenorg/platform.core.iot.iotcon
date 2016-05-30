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
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <tizen_type.h>

#include <iotcon.h>
#include "test.h"

static void _register_unowned_devices(iotcon_provisioning_devices_h owned_devices,
		iotcon_provisioning_devices_h unowned_devices, void *user_data)
{
	FN_CALL;

}


static void _found_devices(iotcon_provisioning_devices_h owned_devices,
		iotcon_provisioning_devices_h unowned_devices, void *user_data)
{
	FN_CALL;
	int ret;
	iotcon_provisioning_devices_h cloned_devices;

	if (NULL == unowned_devices) {
		ERR("No unowned devices");
		return;
	}

	ret = iotcon_provisioning_devices_clone(unowned_devices, &cloned_devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_devices_clone() Fail(%d)", ret);
		return;
	}

	ret = iotcon_provisioning_register_unowned_devices(cloned_devices,
			_register_unowned_devices, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_register_unowned_devices() Fail(%d)", ret);
		return;
	}

}


static void _randompins_cb(char *pin, int len, void *user_data)
{
	scanf("%s", pin);

}


int main()
{
	int ret;
	int timeout = 5;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	ret = iotcon_provisioning_init("/usr/bin/iotcon-test-svr-db.dat",
			"/usr/bin/iotcon-test-prvn-mng.db");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_init() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_provisioning_set_randompins(_randompins_cb, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_set_randompins() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_provisioning_discover_all_devices(timeout, _found_devices, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_discover_all_devices() Fail(%d)", ret);
		return -1;
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	return 0;
}
