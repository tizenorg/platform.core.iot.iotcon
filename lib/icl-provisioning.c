/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <octypes.h>
#include <ocstack.h>
#include <ocprovisioningmanager.h>
#include <oxmjustworks.h>
#include <oxmrandompin.h>
#include <pmutility.h>

#include "iotcon.h"
#include "iotcon-provisioning.h"
#include "ic-utils.h"
#include "ic-ioty-types.h"
#include "icl.h"
#include "icl-ioty.h"
#include "icl-provisioning-struct.h"

#define ICL_PROVISIONING_TIMEOUT_MAX 10

typedef enum {
	ICL_PROVISIONING_DISCOVER_ALL_DEVICES,
	ICL_PROVISIONING_DISCOVER_OWNED_DEVICES,
	ICL_PROVISIONING_DISCOVER_UNOWNED_DEVICES,
} icl_provisioning_discover_e;

struct icl_provisioning_randompins_cb_container {
	iotcon_provisioning_randompins_cb cb;
	void *user_data;
};

struct icl_provisioning_ownership_transfer_cb_container {
	iotcon_provisioning_devices_h owned_devices;
	iotcon_provisioning_devices_h unowned_devices;
	iotcon_provisioning_ownership_transfer_cb cb;
	void *user_data;
};

typedef struct {
	int timeout;
	iotcon_provisioning_found_devices_cb cb;
	void *user_data;
	iotcon_provisioning_devices_h owned_devices;
	iotcon_provisioning_devices_h unowned_devices;
} icl_provisioning_discover_cb_container_s;

static OTMCallbackData_t icl_justworks_otmcb;
static OTMCallbackData_t icl_pinbased_otmcb;
static struct icl_provisioning_randompins_cb_container icl_randompins_cb_container;

static iotcon_provisioning_devices_h icl_owned_devices;
static iotcon_provisioning_devices_h icl_unowned_devices;


static void _provisioning_set_justworks()
{
	icl_justworks_otmcb.loadSecretCB = LoadSecretJustWorksCallback;
	icl_justworks_otmcb.createSecureSessionCB = CreateSecureSessionJustWorksCallback;
	icl_justworks_otmcb.createSelectOxmPayloadCB = CreateJustWorksSelectOxmPayload;
	icl_justworks_otmcb.createOwnerTransferPayloadCB = CreateJustWorksOwnerTransferPayload;
}


static void _provisioning_set_randompins()
{
	icl_pinbased_otmcb.loadSecretCB = InputPinCodeCallback;
	icl_pinbased_otmcb.createSecureSessionCB = CreateSecureSessionRandomPinCallback;
	icl_pinbased_otmcb.createSelectOxmPayloadCB = CreatePinBasedSelectOxmPayload;
	icl_pinbased_otmcb.createOwnerTransferPayloadCB = CreatePinBasedOwnerTransferPayload;
}


API int iotcon_provisioning_init(const char *file_path, const char *db_path)
{
	FN_CALL;
	int ret;
	OCStackResult result;

	RETV_IF(false == ic_utils_check_oic_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(false == ic_utils_check_permission((IC_PERMISSION_INTERNET
					|IC_PERMISSION_NETWORK_GET)), IOTCON_ERROR_PERMISSION_DENIED);
	RETV_IF(NULL == file_path, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_ioty_set_persistent_storage(file_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_set_persistent_storage() Fail(%d)", ret);
		return ret;
	}

	ret = iotcon_initialize();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_initialize() Fail(%d)", ret);
		return ret;
	}

	ret = access(db_path, F_OK);
	if (-1 != ret)
		DBG("Provisioning DB File already exists.");
	else
		DBG("No provisioning DB File, creating new.");

	result = OCInitPM(db_path);
	if (OC_STACK_OK != result) {
		ERR("OCInitPM() Fail(%d)", result);
		return ic_ioty_parse_oic_error(result);
	}

	_provisioning_set_justworks();
	result = OCSetOwnerTransferCallbackData(OIC_JUST_WORKS, &icl_justworks_otmcb);
	if (OC_STACK_OK != result) {
		ERR("OCSetOwnerTransferCallbackData() Fail(%d)", result);
		return ic_ioty_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_input_pin_cb(char *pin, size_t len)
{
	FN_CALL;

	RET_IF(NULL == icl_randompins_cb_container.cb);
	RET_IF(NULL == pin);
	RET_IF(len <= OXM_RANDOM_PIN_SIZE);

	icl_randompins_cb_container.cb(pin, len, icl_randompins_cb_container.user_data);
}


API int iotcon_provisioning_set_randompins(iotcon_provisioning_randompins_cb cb,
		void *user_data)
{
	FN_CALL;
	OCStackResult result;

	RETV_IF(false == ic_utils_check_oic_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	_provisioning_set_randompins();
	result = OCSetOwnerTransferCallbackData(OIC_RANDOM_DEVICE_PIN, &icl_pinbased_otmcb);
	if (OC_STACK_OK != result) {
		ERR("OCSetOwnerTransferCallbackData() Fail(%d)", result);
		return ic_ioty_parse_oic_error(result);
	}

	icl_randompins_cb_container.cb = cb;
	icl_randompins_cb_container.user_data = user_data;

	SetInputPinCB(_provisioning_input_pin_cb);

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_get_devices(iotcon_provisioning_devices_h *owned_devices,
		iotcon_provisioning_devices_h *unowned_devices)
{
	FN_CALL;

	if (owned_devices)
		*owned_devices = icl_owned_devices;

	if (unowned_devices)
		*unowned_devices = icl_unowned_devices;

	return IOTCON_ERROR_NONE;
}


static void _provisioning_discover_cb_container_destroy(
		icl_provisioning_discover_cb_container_s *container)
{
	if (container->owned_devices) {
		iotcon_provisioning_devices_destroy(container->owned_devices);
		container->owned_devices = NULL;
	}

	if (container->unowned_devices) {
		iotcon_provisioning_devices_destroy(container->unowned_devices);
		container->unowned_devices = NULL;
	}

	free(container);
}


static gboolean _provisioning_discover_idle_cb(gpointer p)
{
	icl_provisioning_discover_cb_container_s *container = p;

	if (container->cb)
		container->cb(container->owned_devices, container->unowned_devices,
				container->user_data);

	_provisioning_discover_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void* _provisioning_discover_all_thread(void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	OCProvisionDev_t *owned_list = NULL;
	OCProvisionDev_t *unowned_list = NULL;
	icl_provisioning_discover_cb_container_s *container = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCGetDevInfoFromNetwork(container->timeout, &owned_list, &unowned_list);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCGetDevInfoFromNetwork() Fail(%d)", result);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	ret = icl_provisioning_devices_create(owned_list, &container->owned_devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_devices_create() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	ret = icl_provisioning_devices_create(unowned_list, &container->unowned_devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_devices_create() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	icl_owned_devices = container->owned_devices;
	icl_unowned_devices = container->unowned_devices;

	g_idle_add(_provisioning_discover_idle_cb, container);

	return NULL;
}


static void* _provisioning_discover_unowned_thread(void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	OCProvisionDev_t *unowned_list = NULL;
	icl_provisioning_discover_cb_container_s *container = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCDiscoverUnownedDevices(container->timeout, &unowned_list);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCDiscoverUnownedDevices() Fail(%d)", result);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	ret = icl_provisioning_devices_create(unowned_list, &container->unowned_devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_devices_create() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	icl_unowned_devices = container->unowned_devices;

	g_idle_add(_provisioning_discover_idle_cb, container);

	return NULL;
}


static void* _provisioning_discover_owned_thread(void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	OCProvisionDev_t *owned_list = NULL;
	icl_provisioning_discover_cb_container_s *container = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCDiscoverOwnedDevices(container->timeout, &owned_list);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCDiscoverOwnedDevices() Fail(%d)", result);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	ret = icl_provisioning_devices_create(owned_list, &container->owned_devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_devices_create() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	icl_owned_devices = container->owned_devices;

	g_idle_add(_provisioning_discover_idle_cb, container);

	return NULL;
}


static int _provisioning_discover_thread(int type, void *user_data)
{
	int ret;
	pthread_t thread;
	void *thread_routine;

	switch (type) {
	case ICL_PROVISIONING_DISCOVER_ALL_DEVICES:
		thread_routine = _provisioning_discover_all_thread;
		break;
	case ICL_PROVISIONING_DISCOVER_OWNED_DEVICES:
		thread_routine = _provisioning_discover_owned_thread;
		break;
	case ICL_PROVISIONING_DISCOVER_UNOWNED_DEVICES:
		thread_routine = _provisioning_discover_unowned_thread;
		break;
	default:
		ERR("Invalid Type(%d)", type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	ret = pthread_create(&thread, NULL, thread_routine, user_data);
	if (0 != ret) {
		ERR("pthread_create() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_discover_all_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	icl_provisioning_discover_cb_container_s *container;

	RETV_IF(false == ic_utils_check_oic_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(0 > timeout || ICL_PROVISIONING_TIMEOUT_MAX >= timeout,
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	INFO("Discover All Devices");

	container = calloc(1, sizeof(icl_provisioning_discover_cb_container_s));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->timeout = timeout;
	container->cb = cb;
	container->user_data = user_data;

	ret = _provisioning_discover_thread(ICL_PROVISIONING_DISCOVER_ALL_DEVICES, container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_discover_thread() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_discover_unowned_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	icl_provisioning_discover_cb_container_s *container;

	RETV_IF(false == ic_utils_check_oic_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(0 > timeout || ICL_PROVISIONING_TIMEOUT_MAX >= timeout,
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	INFO("Discover Unowned Devices");

	container = calloc(1, sizeof(icl_provisioning_discover_cb_container_s));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->timeout = timeout;
	container->cb = cb;
	container->user_data = user_data;

	ret = _provisioning_discover_thread(ICL_PROVISIONING_DISCOVER_UNOWNED_DEVICES,
			container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_discover_thread() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_discover_owned_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	icl_provisioning_discover_cb_container_s *container;

	RETV_IF(false == ic_utils_check_oic_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(0 > timeout || ICL_PROVISIONING_TIMEOUT_MAX >= timeout,
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	INFO("Discover Owned Devices");

	container = calloc(1, sizeof(icl_provisioning_discover_cb_container_s));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->timeout = timeout;
	container->cb = cb;
	container->user_data = user_data;

	ret = _provisioning_discover_thread(ICL_PROVISIONING_DISCOVER_OWNED_DEVICES,
			container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_discover_thread() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_ownership_transfer_cb_container_destroy(
		struct icl_provisioning_ownership_transfer_cb_container *container)
{
	if (container->owned_devices) {
		iotcon_provisioning_devices_destroy(container->owned_devices);
		container->owned_devices = NULL;
	}

	if (container->unowned_devices) {
		iotcon_provisioning_devices_destroy(container->unowned_devices);
		container->unowned_devices = NULL;
	}

	free(container);
}


static gboolean _provisioning_ownership_transfer_idle_cb(gpointer p)
{
	struct icl_provisioning_ownership_transfer_cb_container *container = p;

	if (container->cb)
		container->cb(container->owned_devices, container->unowned_devices,
				container->user_data);

	_provisioning_ownership_transfer_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void _provisioning_ownership_transfer_cb(void *ctx, int n_of_res,
		OCProvisionResult_t *arr, bool has_error)
{
	FN_CALL;
	int i, ret;
	struct icl_provisioning_ownership_transfer_cb_container *container;
	iotcon_provisioning_devices_h owned_devices;
	iotcon_provisioning_devices_h unowned_devices;

	container = ctx;

	if (true == has_error) {
		ERR("ownership transfer Fail");
		_provisioning_ownership_transfer_cb_container_destroy(container);
		return;
	}

	ret = icl_provisioning_devices_create(NULL, &owned_devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_devices_create() Fail(%d)", ret);
		_provisioning_ownership_transfer_cb_container_destroy(container);
		return;
	}

	unowned_devices = container->unowned_devices;

	for (i = 0; i < n_of_res; i++) {
		if (0 == arr[i].res) {
			icl_provisioning_devices_move_device((OicUuid_t*)&arr[i].deviceId,
					unowned_devices, owned_devices);
		}
	}

	g_idle_add(_provisioning_ownership_transfer_idle_cb, container);

	return;
}


API int iotcon_provisioning_register_unowned_devices(
		iotcon_provisioning_devices_h devices,
		iotcon_provisioning_ownership_transfer_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;
	OCProvisionDev_t *dev_list;
	struct icl_provisioning_ownership_transfer_cb_container *container;

	RETV_IF(false == ic_utils_check_oic_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	container = calloc(1, sizeof(struct icl_provisioning_ownership_transfer_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->unowned_devices = devices;
	container->cb = cb;
	container->user_data = user_data;

	dev_list = icl_provisioning_devices_get_devices(devices);

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	ret = OCDoOwnershipTransfer(container, dev_list, _provisioning_ownership_transfer_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != ret) {
		ERR("OCDoOwnershipTransfer() Fail(%d)", ret);
		_provisioning_ownership_transfer_cb_container_destroy(container);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}


