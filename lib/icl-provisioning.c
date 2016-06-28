/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
	ICL_PROVISIONING_REMOVE_DEVICE,
} icl_provisioning_discover_e;

struct icl_provisioning_randompins_cb_container {
	iotcon_provisioning_randompins_cb cb;
	void *user_data;
};

struct icl_provisioning_discover_cb_container {
	int timeout;
	iotcon_provisioning_found_devices_cb cb;
	void *user_data;
	iotcon_provisioning_devices_h owned_devices;
	iotcon_provisioning_devices_h unowned_devices;
};

struct icl_provisioning_ownership_transfer_cb_container {
	int count;
	OCProvisionResult_t *result_list;
	iotcon_provisioning_devices_h devices;
	iotcon_provisioning_ownership_transfer_cb cb;
	void *user_data;
};

struct icl_provisioning_provision_cred_cb_container {
	iotcon_provisioning_device_h device1;
	iotcon_provisioning_device_h device2;
	iotcon_error_e result;
	iotcon_provisioning_provision_cred_cb cb;
	void *user_data;
};

struct icl_provisioning_provision_acl_cb_container {
	iotcon_provisioning_device_h device;
	iotcon_provisioning_acl_h acl;
	iotcon_error_e result;
	iotcon_provisioning_provision_acl_cb cb;
	void *user_data;
};

struct icl_provisioning_pairwise_devices_cb_container {
	iotcon_provisioning_device_h device1;
	iotcon_provisioning_acl_h acl1;
	iotcon_provisioning_device_h device2;
	iotcon_provisioning_acl_h acl2;
	iotcon_error_e result;
	iotcon_provisioning_pairwise_devices_cb cb;
	void *user_data;
};

struct icl_provisioning_unlink_pairwise_cb_container {
	iotcon_provisioning_device_h device1;
	iotcon_provisioning_device_h device2;
	iotcon_error_e result;
	iotcon_provisioning_unlink_pairwise_cb cb;
	void *user_data;
};

struct icl_provisioning_remove_device_cb_container {
	int timeout;
	iotcon_provisioning_device_h device;
	iotcon_error_e result;
	iotcon_provisioning_remove_device_cb cb;
	void *user_data;
};

static OTMCallbackData_t icl_justworks_otmcb;
static OTMCallbackData_t icl_pinbased_otmcb;
static struct icl_provisioning_randompins_cb_container icl_randompins_cb_container;

static iotcon_provisioning_devices_h icl_owned_devices;
static iotcon_provisioning_devices_h icl_unowned_devices;

static void* _provisioning_remove_device_thread(void *user_data);

static iotcon_error_e _provisioning_parse_oic_error(OCStackResult ret)
{
	switch (ret) {
	case OC_STACK_RESOURCE_CREATED:
	case OC_STACK_RESOURCE_DELETED:
		return IOTCON_ERROR_NONE;
	case OC_STACK_AUTHENTICATION_FAILURE:
		return IOTCON_ERROR_AUTHENTICATION_FAILURE;
	default:
		return ic_ioty_parse_oic_error(ret);
	}
}

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


API int iotcon_provisioning_initialize(const char *file_path, const char *db_path)
{
	FN_CALL;
	int ret;
	OCStackResult result;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(false == icl_check_init(), IOTCON_ERROR_IO_ERROR);
	RETV_IF(false == ic_utils_check_permission((IC_PERMISSION_INTERNET
					|IC_PERMISSION_NETWORK_GET)), IOTCON_ERROR_PERMISSION_DENIED);
	RETV_IF(NULL == file_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == db_path, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_initialize(file_path, true);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_initialize() Fail(%d)", ret);
		return ret;
	}

	ret = access(db_path, R_OK | W_OK);
	if (-1 != ret)
		DBG("Provisioning DB File already exists.");
	else
		DBG("No provisioning DB File, creating new.");

	result = OCInitPM(db_path);
	if (OC_STACK_OK != result) {
		ERR("OCInitPM() Fail(%d)", result);
		return _provisioning_parse_oic_error(result);
	}

	_provisioning_set_justworks();
	result = OCSetOwnerTransferCallbackData(OIC_JUST_WORKS, &icl_justworks_otmcb);
	if (OC_STACK_OK != result) {
		ERR("OCSetOwnerTransferCallbackData() Fail(%d)", result);
		return _provisioning_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_input_pin_cb(char *pin, size_t len)
{
	FN_CALL;
	char *temp;

	RET_IF(NULL == icl_randompins_cb_container.cb);
	RET_IF(NULL == pin);
	RET_IF(len <= OXM_RANDOM_PIN_SIZE);

	temp = icl_randompins_cb_container.cb(icl_randompins_cb_container.user_data);
	if ((NULL == temp) || (len <= strlen(temp))) {
		ERR("Invalid Pin Number");
		return;
	}

	strncpy(pin, temp, len);
}


API int iotcon_provisioning_set_randompins(iotcon_provisioning_randompins_cb cb,
		void *user_data)
{
	FN_CALL;
	OCStackResult result;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	_provisioning_set_randompins();
	result = OCSetOwnerTransferCallbackData(OIC_RANDOM_DEVICE_PIN, &icl_pinbased_otmcb);
	if (OC_STACK_OK != result) {
		ERR("OCSetOwnerTransferCallbackData() Fail(%d)", result);
		return _provisioning_parse_oic_error(result);
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

	RETV_IF(NULL == owned_devices && NULL == unowned_devices,
			IOTCON_ERROR_INVALID_PARAMETER);

	if (owned_devices)
		*owned_devices = icl_owned_devices;

	if (unowned_devices)
		*unowned_devices = icl_unowned_devices;

	return IOTCON_ERROR_NONE;
}


static void _provisioning_discover_cb_container_destroy(
		struct icl_provisioning_discover_cb_container *container)
{
	FN_CALL;

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
	FN_CALL;
	struct icl_provisioning_discover_cb_container *container = p;

	DBG("discovered owned devices");
	icl_provisioning_devices_print_uuid(container->owned_devices);
	DBG("discovered unowned devices");
	icl_provisioning_devices_print_uuid(container->unowned_devices);

	icl_provisioning_devices_set_found(container->owned_devices);
	icl_provisioning_devices_set_found(container->unowned_devices);

	if (container->cb)
		container->cb(container->owned_devices, container->unowned_devices,
				container->user_data);

	icl_provisioning_devices_unset_found(container->unowned_devices);
	icl_provisioning_devices_unset_found(container->owned_devices);

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
	iotcon_provisioning_devices_h temp_devices;
	struct icl_provisioning_discover_cb_container *container = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCGetDevInfoFromNetwork(container->timeout, &owned_list, &unowned_list);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCGetDevInfoFromNetwork() Fail(%d)", result);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	if (owned_list) {
		ret = icl_provisioning_devices_create(owned_list, &container->owned_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_provisioning_devices_create() Fail(%d)", ret);
			_provisioning_discover_cb_container_destroy(container);
			return NULL;
		}

		ret = iotcon_provisioning_devices_clone(container->owned_devices, &temp_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_provisioning_devices_clone() Fail(%d)", ret);
			return NULL;
		}

		if (icl_owned_devices) {
			icl_provisioning_devices_unset_found(icl_owned_devices);
			iotcon_provisioning_devices_destroy(icl_owned_devices);
		}
		icl_owned_devices = temp_devices;
		icl_provisioning_devices_set_found(icl_owned_devices);
	}

	if (unowned_list) {
		ret = icl_provisioning_devices_create(unowned_list, &container->unowned_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_provisioning_devices_create() Fail(%d)", ret);
			_provisioning_discover_cb_container_destroy(container);
			return NULL;
		}

		ret = iotcon_provisioning_devices_clone(container->unowned_devices, &temp_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_provisioning_devices_clone() Fail(%d)", ret);
			return NULL;
		}

		if (icl_unowned_devices) {
			icl_provisioning_devices_unset_found(icl_unowned_devices);
			iotcon_provisioning_devices_destroy(icl_unowned_devices);
		}
		icl_unowned_devices = temp_devices;
		icl_provisioning_devices_set_found(icl_unowned_devices);
	}

	g_idle_add(_provisioning_discover_idle_cb, container);

	return NULL;
}


static void* _provisioning_discover_unowned_thread(void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	OCProvisionDev_t *unowned_list = NULL;
	iotcon_provisioning_devices_h temp_devices;
	struct icl_provisioning_discover_cb_container *container = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCDiscoverUnownedDevices(container->timeout, &unowned_list);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCDiscoverUnownedDevices() Fail(%d)", result);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	if (unowned_list) {
		ret = icl_provisioning_devices_create(unowned_list, &container->unowned_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_provisioning_devices_create() Fail(%d)", ret);
			_provisioning_discover_cb_container_destroy(container);
			return NULL;
		}

		ret = iotcon_provisioning_devices_clone(container->unowned_devices, &temp_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_provisioning_devices_clone() Fail(%d)", ret);
			return NULL;
		}

		if (icl_unowned_devices) {
			icl_provisioning_devices_unset_found(icl_unowned_devices);
			iotcon_provisioning_devices_destroy(icl_unowned_devices);
		}
		icl_unowned_devices = temp_devices;
		icl_provisioning_devices_set_found(icl_unowned_devices);
	}

	g_idle_add(_provisioning_discover_idle_cb, container);

	return NULL;
}


static void* _provisioning_discover_owned_thread(void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	OCProvisionDev_t *owned_list = NULL;
	iotcon_provisioning_devices_h temp_devices;
	struct icl_provisioning_discover_cb_container *container = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCDiscoverOwnedDevices(container->timeout, &owned_list);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCDiscoverOwnedDevices() Fail(%d)", result);
		_provisioning_discover_cb_container_destroy(container);
		return NULL;
	}

	if (owned_list) {
		ret = icl_provisioning_devices_create(owned_list, &container->owned_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_provisioning_devices_create() Fail(%d)", ret);
			_provisioning_discover_cb_container_destroy(container);
			return NULL;
		}

		ret = iotcon_provisioning_devices_clone(container->owned_devices, &temp_devices);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_provisioning_devices_clone() Fail(%d)", ret);
			return NULL;
		}

		if (icl_owned_devices) {
			icl_provisioning_devices_unset_found(icl_owned_devices);
			iotcon_provisioning_devices_destroy(icl_owned_devices);
		}
		icl_owned_devices = temp_devices;
		icl_provisioning_devices_set_found(icl_owned_devices);
	}

	g_idle_add(_provisioning_discover_idle_cb, container);

	return NULL;
}


static int _provisioning_thread(int type, void *user_data)
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
	case ICL_PROVISIONING_REMOVE_DEVICE:
		thread_routine = _provisioning_remove_device_thread;
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
	pthread_detach(thread);

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_find_all_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	struct icl_provisioning_discover_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(timeout < 0 || ICL_PROVISIONING_TIMEOUT_MAX < timeout,
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	INFO("Discover All Devices");

	container = calloc(1, sizeof(struct icl_provisioning_discover_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->timeout = timeout;
	container->cb = cb;
	container->user_data = user_data;

	ret = _provisioning_thread(ICL_PROVISIONING_DISCOVER_ALL_DEVICES, container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_thread() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_find_unowned_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	struct icl_provisioning_discover_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(timeout < 0 || ICL_PROVISIONING_TIMEOUT_MAX < timeout,
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	INFO("Discover Unowned Devices");

	container = calloc(1, sizeof(struct icl_provisioning_discover_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->timeout = timeout;
	container->cb = cb;
	container->user_data = user_data;

	ret = _provisioning_thread(ICL_PROVISIONING_DISCOVER_UNOWNED_DEVICES,
			container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_thread() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_find_owned_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	struct icl_provisioning_discover_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(timeout < 0 || ICL_PROVISIONING_TIMEOUT_MAX < timeout,
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	INFO("Discover Owned Devices");

	container = calloc(1, sizeof(struct icl_provisioning_discover_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->timeout = timeout;
	container->cb = cb;
	container->user_data = user_data;

	ret = _provisioning_thread(ICL_PROVISIONING_DISCOVER_OWNED_DEVICES,
			container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_thread() Fail(%d)", ret);
		_provisioning_discover_cb_container_destroy(container);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_ownership_transfer_cb_container_destroy(
		struct icl_provisioning_ownership_transfer_cb_container *container)
{
	if (container->devices) {
		iotcon_provisioning_devices_destroy(container->devices);
		container->devices = NULL;
	}

	free(container->result_list);
	free(container);
}


static int _provisioning_ownership_transfer_get_result(
		iotcon_provisioning_device_h device, OCProvisionResult_t *result_list, int count)
{
	int i;
	OCProvisionDev_t *oic_device;

	oic_device = icl_provisioning_device_get_device(device);

	for (i = 0; i < count; i++) {
		if (true == icl_provisioning_compare_oic_uuid(&oic_device->doxm->deviceID,
					(OicUuid_t*)&result_list[i].deviceId))
			return _provisioning_parse_oic_error(result_list[i].res);
	}

	return IOTCON_ERROR_IOTIVITY;
}


static bool _provisioning_ownership_transfer_foreach_cb(
		iotcon_provisioning_devices_h devices,
		iotcon_provisioning_device_h device,
		void *user_data)
{
	int result;
	struct icl_provisioning_ownership_transfer_cb_container *container = user_data;

	result = _provisioning_ownership_transfer_get_result(device, container->result_list,
			container->count);

	icl_provisioning_device_set_found(device);

	if (container->cb)
		container->cb(device, result, container->user_data);

	icl_provisioning_device_unset_found(device);

	return IOTCON_FUNC_CONTINUE;
}


static gboolean _provisioning_ownership_transfer_idle_cb(gpointer p)
{
	int ret;
	struct icl_provisioning_ownership_transfer_cb_container *container = p;

	ret = iotcon_provisioning_devices_foreach(container->devices,
			_provisioning_ownership_transfer_foreach_cb, container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_devices_foreach() Fail(%d)", ret);
		if (container->cb)
			container->cb(NULL, ret, container->user_data);
	}

	_provisioning_ownership_transfer_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void _provisioning_ownership_transfer_cb(void *ctx, int n_of_res,
		OCProvisionResult_t *arr, bool has_error)
{
	FN_CALL;
	struct icl_provisioning_ownership_transfer_cb_container *container;
	OCProvisionResult_t *result_list;

	container = ctx;
	container->count = n_of_res;

	result_list = calloc(n_of_res + 1, sizeof(OCProvisionResult_t));
	if (NULL == result_list) {
		ERR("calloc() Fail(%d)", errno);
		return;
	}
	memcpy(result_list, arr, n_of_res * sizeof(OCProvisionResult_t));

	container->result_list = result_list;

	if (true == has_error)
		DBG("ownership transfer has error");

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
	OCStackResult result;
	OCProvisionDev_t *dev_list;
	struct icl_provisioning_ownership_transfer_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	container = calloc(1, sizeof(struct icl_provisioning_ownership_transfer_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = iotcon_provisioning_devices_clone(devices, &container->devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_devices_clone() Fail(%d)", ret);
		_provisioning_ownership_transfer_cb_container_destroy(container);
		return ret;
	}

	container->cb = cb;
	container->user_data = user_data;

	dev_list = icl_provisioning_devices_get_devices(devices);

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCDoOwnershipTransfer(container, dev_list, _provisioning_ownership_transfer_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCDoOwnershipTransfer() Fail(%d)", result);
		_provisioning_ownership_transfer_cb_container_destroy(container);
		return _provisioning_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_register_unowned_device(
		iotcon_provisioning_device_h device,
		iotcon_provisioning_ownership_transfer_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	OCProvisionDev_t *dev_list, *cloned_list;
	struct icl_provisioning_ownership_transfer_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	container = calloc(1, sizeof(struct icl_provisioning_ownership_transfer_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->cb = cb;
	container->user_data = user_data;

	dev_list = icl_provisioning_device_get_device(device);
	cloned_list = icl_provisioning_devices_clone(dev_list);

	ret = icl_provisioning_devices_create(cloned_list, &container->devices);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_devices_create() Fail(%d)", ret);
		return ret;
	}

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCDoOwnershipTransfer(container, cloned_list, _provisioning_ownership_transfer_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCDoOwnershipTransfer() Fail(%d)", result);
		_provisioning_ownership_transfer_cb_container_destroy(container);
		return _provisioning_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_provision_cred_cb_container_destroy(
		struct icl_provisioning_provision_cred_cb_container *container)
{
	FN_CALL;

	if (container->device1) {
		iotcon_provisioning_device_destroy(container->device1);
		container->device1 = NULL;
	}
	if (container->device2) {
		iotcon_provisioning_device_destroy(container->device2);
		container->device2 = NULL;
	}

	free(container);
}


static gboolean _provisioning_provision_cred_idle_cb(gpointer p)
{
	struct icl_provisioning_provision_cred_cb_container *container = p;

	if (container->cb) {
		container->cb(container->device1, container->device2, container->result,
				container->user_data);
	}

	_provisioning_provision_cred_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void _provisioning_provision_cred_cb(void *ctx, int n_of_res,
		OCProvisionResult_t *arr, bool has_error)
{
	FN_CALL;
	int i;
	struct icl_provisioning_provision_cred_cb_container *container = ctx;

	if (true == has_error)
		DBG("provision cred has error");

	container->result = IOTCON_ERROR_NONE;

	for (i = 0; i < n_of_res; i++) {
		DBG("arr[%d].res : %d", i, arr[i].res);
		if (OC_STACK_OK != arr[i].res)
			container->result = _provisioning_parse_oic_error(arr[i].res);
	}
	g_idle_add(_provisioning_provision_cred_idle_cb, container);
}


API int iotcon_provisioning_provision_cred(iotcon_provisioning_device_h device1,
		iotcon_provisioning_device_h device2,
		iotcon_provisioning_provision_cred_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;
	size_t key_size;
	OCStackResult result;
	struct icl_provisioning_provision_cred_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device1, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device2, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	key_size = OWNER_PSK_LENGTH_256; /* or OWNER_PSK_LENGTH_128 */

	container = calloc(1, sizeof(struct icl_provisioning_provision_cred_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = iotcon_provisioning_device_clone(device1, &container->device1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_provision_cred_cb_container_destroy(container);
		return ret;
	}
	ret = iotcon_provisioning_device_clone(device2, &container->device2);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_provision_cred_cb_container_destroy(container);
		return ret;
	}
	container->cb = cb;
	container->user_data = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCProvisionCredentials(container,
			SYMMETRIC_PAIR_WISE_KEY,
			key_size,
			icl_provisioning_device_get_device(container->device1),
			icl_provisioning_device_get_device(container->device2),
			_provisioning_provision_cred_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCProvisionCredentails() Fail(%d)", result);
		_provisioning_provision_cred_cb_container_destroy(container);
		return _provisioning_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_provision_acl_cb_container_destroy(
		struct icl_provisioning_provision_acl_cb_container *container)
{
	FN_CALL;

	if (container->device) {
		iotcon_provisioning_device_destroy(container->device);
		container->device = NULL;
	}

	free(container);
}



static char* _provisioning_parse_uuid(OicUuid_t *uuid)
{
	char uuid_string[256] = {0};

	snprintf(uuid_string, sizeof(uuid_string),
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			(*uuid).id[0], (*uuid).id[1], (*uuid).id[2], (*uuid).id[3],
			(*uuid).id[4], (*uuid).id[5], (*uuid).id[6], (*uuid).id[7],
			(*uuid).id[8], (*uuid).id[9], (*uuid).id[10], (*uuid).id[11],
			(*uuid).id[12], (*uuid).id[13], (*uuid).id[14], (*uuid).id[15]);

	DBG("uuid : %s", uuid_string);

	return strdup(uuid_string);
}


static OicSecAcl_t* _provisioning_convert_acl(iotcon_provisioning_device_h device,
		iotcon_provisioning_acl_h acl)
{
	int i;
	int permission;
	char *uri_path;
	OicSecAcl_t *oic_acl;
	OCProvisionDev_t *subject, *oic_device;

	oic_acl = calloc(1, sizeof(OicSecAcl_t));
	if (NULL == oic_acl) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	subject = icl_provisioning_acl_get_subject(acl);

	memcpy(&oic_acl->subject, &subject->doxm->deviceID, 128/8);
	memcpy(&oic_acl->rownerID, &subject->doxm->deviceID, sizeof(OicUuid_t));

	_provisioning_parse_uuid(&oic_acl->subject);

	oic_acl->resourcesLen = icl_provisioning_acl_get_resource_count(acl);

	oic_acl->resources = calloc(oic_acl->resourcesLen, sizeof(char *));
	if (NULL == oic_acl->resources) {
		ERR("calloc() Fail(%d)", errno);
		OCDeleteACLList(oic_acl);
		return NULL;
	}
	DBG("resource num : %d", oic_acl->resourcesLen);

	for (i = 0; i < oic_acl->resourcesLen; i++) {
		uri_path = icl_provisioning_acl_get_nth_resource(acl, i);
		if (NULL == uri_path) {
			ERR("icl_provisioning_acl_get_nth_resource() Fail(%d)", errno);
			OCDeleteACLList(oic_acl);
			return NULL;
		}
		oic_acl->resources[i] = ic_utils_strdup(uri_path);
		DBG("resource : (%s)", oic_acl->resources[i]);
	}

	permission = icl_provisioning_acl_get_permission(acl);
	oic_acl->permission = icl_provisioning_acl_convert_permission(permission);

	oic_device = icl_provisioning_device_get_device(device);

	memcpy(&oic_acl->rownerID, &oic_device->doxm->deviceID, sizeof(OicUuid_t));

	_provisioning_parse_uuid(&oic_acl->rownerID);

	return oic_acl;
}


static gboolean _provisioning_provision_acl_idle_cb(void *p)
{
	struct icl_provisioning_provision_acl_cb_container *container = p;

	if (container->cb) {
		container->cb(container->device, container->acl, container->result,
				container->user_data);
	}

	_provisioning_provision_acl_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void _provisioning_provision_acl_cb(void *ctx, int n_of_res,
		OCProvisionResult_t *arr, bool has_error)
{
	FN_CALL;
	int i;
	struct icl_provisioning_provision_acl_cb_container *container = ctx;

	if (true == has_error)
		DBG("provision acl has error");

	container->result = IOTCON_ERROR_NONE;

	for (i = 0; i < n_of_res; i++) {
		DBG("arr[%d].res : %d", i, arr[i].res);
		if (OC_STACK_OK != arr[i].res)
			container->result = _provisioning_parse_oic_error(arr[i].res);
	}
	g_idle_add(_provisioning_provision_acl_idle_cb, container);
}


API int iotcon_provisioning_provision_acl(iotcon_provisioning_device_h device,
		iotcon_provisioning_acl_h acl,
		iotcon_provisioning_provision_acl_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;
	OicSecAcl_t *oic_acl;
	OCStackResult result;
	struct icl_provisioning_provision_acl_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	container = calloc(1, sizeof(struct icl_provisioning_provision_acl_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = iotcon_provisioning_device_clone(device, &container->device);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_provision_acl_cb_container_destroy(container);
		return ret;
	}

	ret = icl_provisioning_acl_clone(acl, &container->acl);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_acl_clone() Fail(%d)", ret);
		_provisioning_provision_acl_cb_container_destroy(container);
		return ret;
	}

	container->cb = cb;
	container->user_data = user_data;

	oic_acl = _provisioning_convert_acl(device, acl);

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCProvisionACL(container,
			icl_provisioning_device_get_device(container->device),
			oic_acl,
			_provisioning_provision_acl_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCProvisionACL() Fail(%d)", result);
		_provisioning_provision_acl_cb_container_destroy(container);
		return _provisioning_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_pairwise_devices_cb_container_destroy(
		struct icl_provisioning_pairwise_devices_cb_container *container)
{
	FN_CALL;

	if (container->device1) {
		iotcon_provisioning_device_destroy(container->device1);
		container->device1 = NULL;
	}
	if (container->acl1) {
		iotcon_provisioning_acl_destroy(container->acl1);
		container->acl1 = NULL;
	}
	if (container->device2) {
		iotcon_provisioning_device_destroy(container->device2);
		container->device2 = NULL;
	}
	if (container->acl2) {
		iotcon_provisioning_acl_destroy(container->acl2);
		container->acl2 = NULL;
	}

	free(container);
}


static gboolean _provisioning_pairwise_devices_idle_cb(void *p)
{
	struct icl_provisioning_pairwise_devices_cb_container *container = p;

	if (container->cb) {
		container->cb(container->device1, container->device2, container->result,
				container->user_data);
	}

	_provisioning_pairwise_devices_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void _provisioning_pairwise_devices_cb(void *ctx, int n_of_res,
		OCProvisionResult_t *arr, bool has_error)
{
	FN_CALL;
	int i;
	struct icl_provisioning_pairwise_devices_cb_container *container = ctx;

	if (true == has_error)
		DBG("pairwise devices has error");

	container->result = IOTCON_ERROR_NONE;

	for (i = 0; i < n_of_res; i++) {
		DBG("arr[%d].res : %d", i, arr[i].res);
		if (OC_STACK_OK != arr[i].res)
			container->result = _provisioning_parse_oic_error(arr[i].res);
	}
	g_idle_add(_provisioning_pairwise_devices_idle_cb, container);
}


API int iotcon_provisioning_pairwise_devices(iotcon_provisioning_device_h device1,
		iotcon_provisioning_acl_h acl1,
		iotcon_provisioning_device_h device2,
		iotcon_provisioning_acl_h acl2,
		iotcon_provisioning_pairwise_devices_cb cb,
		void *user_data)

{
	FN_CALL;
	int ret;
	size_t key_size;
	OCStackResult result;
	OicSecAcl_t *oic_acl1 = NULL;
	OicSecAcl_t *oic_acl2 = NULL;
	struct icl_provisioning_pairwise_devices_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device1, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device2, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	key_size = OWNER_PSK_LENGTH_256; /* or OWNER_PSK_LENGTH_256 */

	container = calloc(1, sizeof(struct icl_provisioning_pairwise_devices_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = iotcon_provisioning_device_clone(device1, &container->device1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_pairwise_devices_cb_container_destroy(container);
		return ret;
	}

	if (acl1) {
		ret = icl_provisioning_acl_clone(acl1, &container->acl1);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_provisioning_acl_clone() Fail(%d)", ret);
			_provisioning_pairwise_devices_cb_container_destroy(container);
			return ret;
		}
		oic_acl1 = _provisioning_convert_acl(container->device1, container->acl1);
	}

	ret = iotcon_provisioning_device_clone(device2, &container->device2);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_pairwise_devices_cb_container_destroy(container);
		return ret;
	}

	if (acl2) {
		ret = icl_provisioning_acl_clone(acl2, &container->acl2);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_provisioning_acl_clone() Fail(%d)", ret);
			_provisioning_pairwise_devices_cb_container_destroy(container);
			return ret;
		}
		oic_acl2 = _provisioning_convert_acl(container->device2, container->acl2);
	}


	container->cb = cb;
	container->user_data = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCProvisionPairwiseDevices(container,
			SYMMETRIC_PAIR_WISE_KEY,
			key_size,
			icl_provisioning_device_get_device(container->device1),
			oic_acl1,
			icl_provisioning_device_get_device(container->device2),
			oic_acl2,
			_provisioning_pairwise_devices_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCProvisionPairwiseDevces() Fail(%d)", result);
		_provisioning_pairwise_devices_cb_container_destroy(container);
		return _provisioning_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_unlink_pairwise_cb_container_destroy(
		struct icl_provisioning_unlink_pairwise_cb_container *container)
{
	if (container->device1) {
		iotcon_provisioning_device_destroy(container->device1);
		container->device1 = NULL;
	}
	if (container->device2) {
		iotcon_provisioning_device_destroy(container->device2);
		container->device2 = NULL;
	}

	free(container);
}


static gboolean _provisioning_unlink_pairwise_idle_cb(gpointer p)
{
	struct icl_provisioning_unlink_pairwise_cb_container *container = p;

	if (container->cb) {
		container->cb(container->device1, container->device2, container->result,
				container->user_data);
	}

	_provisioning_unlink_pairwise_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void _provisioning_unlink_pairwise_cb(void *ctx, int n_of_res,
		OCProvisionResult_t *arr, bool has_error)
{
	FN_CALL;
	int i;
	struct icl_provisioning_unlink_pairwise_cb_container *container = ctx;

	if (true == has_error)
		DBG("unlink pairwise has error");

	for (i = 0; i < n_of_res; i++) {
		DBG("arr[%d].res : %d", i, arr[i].res);
		if (OC_STACK_OK != arr[i].res)
			container->result = _provisioning_parse_oic_error(arr[i].res);
	}

	g_idle_add(_provisioning_unlink_pairwise_idle_cb, container);
}


API int iotcon_provisioning_unlink_pairwise(iotcon_provisioning_device_h device1,
		iotcon_provisioning_device_h device2,
		iotcon_provisioning_unlink_pairwise_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	struct icl_provisioning_unlink_pairwise_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device1, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device2, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	container = calloc(1, sizeof(struct icl_provisioning_unlink_pairwise_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = iotcon_provisioning_device_clone(device1, &container->device1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_unlink_pairwise_cb_container_destroy(container);
		return ret;
	}

	ret = iotcon_provisioning_device_clone(device2, &container->device2);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_unlink_pairwise_cb_container_destroy(container);
		return ret;
	}

	container->cb = cb;
	container->user_data = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCUnlinkDevices(container,
			icl_provisioning_device_get_device(container->device1),
			icl_provisioning_device_get_device(container->device2),
			_provisioning_unlink_pairwise_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCUnlinkDevices() Fail(%d)", result);
		_provisioning_unlink_pairwise_cb_container_destroy(container);
		return _provisioning_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}


static void _provisioning_remove_cb_container_destroy(
		struct icl_provisioning_remove_device_cb_container *container)
{
	if (container->device) {
		iotcon_provisioning_device_destroy(container->device);
		container->device = NULL;
	}

	free(container);
}


static gboolean _provisioning_remove_device_idle_cb(gpointer p)
{
	FN_CALL;
	struct icl_provisioning_remove_device_cb_container *container = p;

	if (container->cb)
		container->cb(container->device, container->result, container->user_data);

	_provisioning_remove_cb_container_destroy(container);

	return G_SOURCE_REMOVE;
}


static void _provisioning_remove_device_cb(void *ctx, int n_of_res,
		OCProvisionResult_t *arr, bool has_error)
{
	FN_CALL;
	struct icl_provisioning_remove_device_cb_container *container = ctx;

	container->result = IOTCON_ERROR_NONE;

	if (true == has_error) {
		DBG("remove device has error");
		container->result = IOTCON_ERROR_IOTIVITY;
	}

	g_idle_add(_provisioning_remove_device_idle_cb, container);
}


static void* _provisioning_remove_device_thread(void *user_data)
{
	FN_CALL;
	OCStackResult result;
	struct icl_provisioning_remove_device_cb_container *container = user_data;

	ic_utils_mutex_lock(IC_UTILS_MUTEX_IOTY);
	result = OCRemoveDevice(container,
			container->timeout,
			icl_provisioning_device_get_device(container->device),
			_provisioning_remove_device_cb);
	ic_utils_mutex_unlock(IC_UTILS_MUTEX_IOTY);
	if (OC_STACK_OK != result) {
		ERR("OCRemoveDevice() Fail(%d)", result);
		_provisioning_remove_cb_container_destroy(container);
		return NULL;
	}

	return NULL;
}


API int iotcon_provisioning_remove_device(int timeout,
		iotcon_provisioning_device_h device,
		iotcon_provisioning_remove_device_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;
	struct icl_provisioning_remove_device_cb_container *container;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(timeout < 0 || ICL_PROVISIONING_TIMEOUT_MAX < timeout,
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	container = calloc(1, sizeof(struct icl_provisioning_remove_device_cb_container));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	container->timeout = timeout;
	container->cb = cb;
	container->user_data = user_data;

	ret = iotcon_provisioning_device_clone(device, &container->device);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_device_clone() Fail(%d)", ret);
		_provisioning_remove_cb_container_destroy(container);
		return ret;
	}

	ret = _provisioning_thread(ICL_PROVISIONING_REMOVE_DEVICE, container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisinoing_discover_thread() Fail(%d)", ret);
		_provisioning_remove_cb_container_destroy(container);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}
