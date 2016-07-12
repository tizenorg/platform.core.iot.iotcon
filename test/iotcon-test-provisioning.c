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

static GList *icl_provisioning_device_list;

static void _provisioning_unlink_pairwise_cb(iotcon_provisioning_device_h device1,
		iotcon_provisioning_device_h device2, iotcon_error_e result, void *user_data)
{
	FN_CALL;

	if (IOTCON_ERROR_NONE != result) {
		ERR("provision unlink pairwise Fail(%d)", result);
		return;
	}

	return;
}

static void _provisioning_provision_acl_cb(iotcon_provisioning_device_h device,
		iotcon_provisioning_acl_h acl, iotcon_error_e result, void *user_data)
{
	FN_CALL;
	int ret;
	iotcon_provisioning_device_h device1, device2;

	if (IOTCON_ERROR_NONE != result) {
		ERR("provision acl Fail(%d)", result);
		return;
	}

	device1 = g_list_nth_data(icl_provisioning_device_list, 0);
	device2 = g_list_nth_data(icl_provisioning_device_list, 1);

	ret = iotcon_provisioning_unlink_pairwise(device1, device2,
			_provisioning_unlink_pairwise_cb, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_unlink_pairwise() Fail(%d)", ret);
		return;
	}
}

static void _provisioning_provision_cred_cb(iotcon_provisioning_device_h device1,
		iotcon_provisioning_device_h device2, iotcon_error_e result, void *user_data)
{
	FN_CALL;
	int ret;
	iotcon_provisioning_acl_h acl;

	if (IOTCON_ERROR_NONE != result) {
		ERR("provision cred Fail(%d)", result);
		return;
	}

	ret = iotcon_provisioning_acl_create(&acl);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_acl_create() Fail(%d)", ret);
		return;
	}

	ret = iotcon_provisioning_acl_set_subject(acl, device1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_acl_set_subject() Fail(%d)", ret);
		iotcon_provisioning_acl_destroy(acl);
		return;
	}

	ret = iotcon_provisioning_acl_add_resource(acl, "/a/door");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_acl_add_resource() Fail(%d)", ret);
		iotcon_provisioning_acl_destroy(acl);
		return;
	}

	ret = iotcon_provisioning_acl_set_permission(acl, IOTCON_PERMISSION_FULL_CONTROL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_acl_set_permission() Fail(%d)", ret);
		iotcon_provisioning_acl_destroy(acl);
		return;
	}

	ret = iotcon_provisioning_provision_acl(device2, acl, _provisioning_provision_acl_cb,
			NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_provision_acl() Fail(%d)", ret);
		iotcon_provisioning_acl_destroy(acl);
		return;
	}

	iotcon_provisioning_acl_destroy(acl);
}

static void _register_unowned_device(iotcon_provisioning_device_h device,
		iotcon_error_e result, void *user_data)
{
	FN_CALL;
	int ret;
	char *device_id;
	char *host_address;
	iotcon_connectivity_type_e connectivity_type;
	iotcon_provisioning_device_h cloned_device;
	iotcon_provisioning_device_h device1, device2;
	bool is_owned;

	ret = iotcon_provisioning_device_get_id(device, &device_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_get_id() Fail(%d)", ret);
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

	ret = iotcon_provisioning_device_clone(device, &cloned_device);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		return;
	}

	DBG("register device(%s) Result : %d", device_id, result);
	DBG(" - host address : %s", host_address);
	DBG(" - connectivity type : %d", connectivity_type);

	icl_provisioning_device_list = g_list_append(icl_provisioning_device_list, cloned_device);

	if (2 == g_list_length(icl_provisioning_device_list)) {
		device1 = g_list_nth_data(icl_provisioning_device_list, 0);
		device2 = g_list_nth_data(icl_provisioning_device_list, 1);

		ret = iotcon_provisioning_provision_cred(device1, device2,
				_provisioning_provision_cred_cb, NULL);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_provisioning_provision_cred() Fail(%d)", ret);
			return;
		}
	}
}


static char* _randompin_cb(void *user_data)
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


bool _found_cb(iotcon_provisioning_device_h device, int result, void *user_data)
{
	FN_CALL;
	int ret;
	bool is_owned;
	char *device_id;
	iotcon_provisioning_device_h cloned_device;

	if (IOTCON_ERROR_NONE != result) {
		ERR("iotcon_provisioning_find_device() Fail(%d)", result);
		return IOTCON_FUNC_CONTINUE;
	}

	ret = iotcon_provisioning_device_is_owned(device, &is_owned);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_is_owned() Fail(%d)", ret);
		return IOTCON_FUNC_CONTINUE;
	}

	ret = iotcon_provisioning_device_get_id(device, &device_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_get_id() Fail(%d)", ret);
		return IOTCON_FUNC_CONTINUE;
	}

	switch (is_owned) {
	case true:
		INFO("device(%s) is owned", device_id);
		break;
	case false:
	default:
		INFO("device(%s) is unowned", device_id);
	}

	if (true == is_owned)
		return IOTCON_FUNC_CONTINUE;

	ret = iotcon_provisioning_device_clone(device, &cloned_device);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		return IOTCON_FUNC_CONTINUE;
	}

	ret = iotcon_provisioning_register_unowned_device(cloned_device,
			_register_unowned_device, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_register_unowned_device() Fail(%d)", ret);
		iotcon_provisioning_device_destroy(cloned_device);
		return IOTCON_FUNC_CONTINUE;
	}

	return IOTCON_FUNC_CONTINUE;
}


int main()
{
	FN_CALL;
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

	ret = iotcon_provisioning_set_randompin_cb(_randompin_cb, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_set_randompin_cb() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_provisioning_find_device(IOTCON_PROVISIONING_FIND_ALL, _found_cb, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_find_device() Fail(%d)", ret);
		return -1;
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	return 0;
}
