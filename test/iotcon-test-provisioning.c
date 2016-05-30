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
#include <iotcon-internal.h>
#include "test.h"

#define IC_MAX_PIN_NUMBER 10

static void _register_unowned_devices(iotcon_provisioning_device_h device,
		iotcon_error_e result, void *user_data)
{
	FN_CALL;
	int ret;
	char *device_id;
	char *host_address;
	int connectivity_type;

	ret = iotcon_provisioning_device_get_device_id(device, &device_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_get_device_id() Fail(%d)", ret);
		return;
	}

	ret = iotcon_provisioning_device_get_host_address(device, &host_address);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_get_host_address() Fail(%d)", ret);
		return;
	}

	ret = iotcon_provisioning_device_get_connectivity_type(device, &connectivity_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_get_connectivity_type() Fail(%d)", ret);
		return;
	}

	DBG("register device(%s) Result : %d", device_id, result);
	DBG(" - host address : %s", host_address);
	DBG(" - connectivity type : %d", connectivity_type);
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


static char* _randompins_cb(void *user_data)
{
	char *pin;

	pin = calloc(1, IC_MAX_PIN_NUMBER);
	if (NULL == pin) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}
	scanf("%s", pin);

	return pin;
}


int main()
{
	int ret;
	int timeout = 5;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	ret = iotcon_provisioning_initialize("/usr/bin/iotcon-test-svr-db.dat",
			"/usr/bin/iotcon-test-prvn-mng.db");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_initialize() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_provisioning_set_randompins(_randompins_cb, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_set_randompins() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_provisioning_find_all_devices(timeout, _found_devices, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_find_all_devices() Fail(%d)", ret);
		return -1;
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	return 0;
}
