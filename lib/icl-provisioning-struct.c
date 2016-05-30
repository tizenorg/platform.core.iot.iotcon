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

struct icl_provisioning_device {
	OCProvisionDev_t *device;
	char *host_address;
	int connectivity_type;
	char *device_id;
};

struct icl_provisioning_devices {
	OCProvisionDev_t *dev_list;
};

struct icl_provisioning_acl {
	OCProvisionDev_t *device;
	GList *resource_list;
	int permission;
};


static char* _provisioning_parse_uuid(OicUuid_t *uuid)
{
	char uuid_string[16] = {0};

	snprintf(uuid_string, sizeof(uuid_string),
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			(*uuid).id[0], (*uuid).id[1], (*uuid).id[2], (*uuid).id[3],
			(*uuid).id[4], (*uuid).id[5], (*uuid).id[6], (*uuid).id[7],
			(*uuid).id[8], (*uuid).id[9], (*uuid).id[10], (*uuid).id[11],
			(*uuid).id[12], (*uuid).id[13], (*uuid).id[14], (*uuid).id[15]);

	return strdup(uuid_string);
}


static int _provisioning_device_create(OCProvisionDev_t *device,
		iotcon_provisioning_device_h *ret_device)
{
	FN_CALL;
	int ret;
	iotcon_provisioning_device_h temp;

	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);

	temp = calloc(1, sizeof(struct icl_provisioning_device));
	if (NULL == temp) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	temp->device = PMCloneOCProvisionDev(device);
	if (NULL == temp->device) {
		ERR("PMCloneOCProvisionDev() Fail(%d)", errno);
		free(temp);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = ic_ioty_parse_oic_dev_address(&device->endpoint, &temp->host_address,
			&temp->connectivity_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_parse_oic_dev_address() Fail(%d)", ret);
		OCDeleteDiscoveredDevices(temp->device);
		free(temp);
		return ret;
	}

	temp->device_id = _provisioning_parse_uuid(&device->doxm->deviceID);

	*ret_device = temp;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_device_clone(iotcon_provisioning_device_h device,
		iotcon_provisioning_device_h *cloned_device)
{
	FN_CALL;
	int ret;
	iotcon_provisioning_device_h temp;

	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cloned_device, IOTCON_ERROR_INVALID_PARAMETER);

	ret = _provisioning_device_create(device->device, &temp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_device_create() Fail(%d)", ret);
		return ret;
	}

	*cloned_device = temp;

	return IOTCON_ERROR_NONE;
}


void iotcon_provisioning_device_destroy(iotcon_provisioning_device_h device)
{
	FN_CALL;

	RET_IF(NULL == device);

	free(device->host_address);
	free(device->device_id);

	OCDeleteDiscoveredDevices(device->device);

	free(device);
}


OCProvisionDev_t* icl_provisioning_device_get_device(
		iotcon_provisioning_device_h device)
{
	FN_CALL;
	RETV_IF(NULL == device, NULL);

	return device->device;
}


int icl_provisioning_devices_create(OCProvisionDev_t *dev_list,
		iotcon_provisioning_devices_h *devices)
{
	FN_CALL;
	iotcon_provisioning_devices_h temp;

	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);

	temp = calloc(1, sizeof(struct icl_provisioning_devices));
	if (NULL == temp) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	temp->dev_list = dev_list;

	*devices = temp;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_devices_create(iotcon_provisioning_devices_h *devices)
{
	FN_CALL;
	int ret;
	iotcon_provisioning_devices_h temp;

	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_provisioning_devices_create(NULL, &temp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_devices_create() Fail(%d)", ret);
		return ret;
	}

	*devices = temp;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_devices_destroy(iotcon_provisioning_devices_h devices)
{
	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);

	if (devices->dev_list)
		OCDeleteDiscoveredDevices(devices->dev_list);

	free(devices);

	return IOTCON_ERROR_NONE;
}


static OCProvisionDev_t* _provisioning_devices_clone(OCProvisionDev_t *src)
{
	OCProvisionDev_t *clone;
	OCProvisionDev_t *current;

	if (NULL == src)
		return NULL;

	clone = PMCloneOCProvisionDev(src);
	if (NULL == clone) {
		ERR("PMCloneOCProvisionDev() Fail");
		return NULL;
	}

	current = clone;
	src = src->next;

	for (; src; src = src->next, current = current->next) {
		current->next = PMCloneOCProvisionDev(src);
		if (NULL == current) {
			ERR("PMCloneOCProvisionDev() Fail");
			OCDeleteDiscoveredDevices(clone);
			return NULL;
		}
	}

	return clone;
}


API int iotcon_provisioning_devices_clone(iotcon_provisioning_devices_h devices,
		iotcon_provisioning_devices_h *cloned_devices)
{
	int ret;
	iotcon_provisioning_devices_h temp;

	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cloned_devices, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_provisioning_devices_create(&temp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_devices_create() Fail(%d)", ret);
		return ret;
	}

	temp->dev_list = _provisioning_devices_clone(devices->dev_list);
	if (NULL == temp->dev_list) {
		ERR("_provisioning_devices_clone() Fail");
		free(temp);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	*cloned_devices = temp;

	return IOTCON_ERROR_NONE;
}



API int iotcon_provisioning_devices_foreach(iotcon_provisioning_devices_h devices,
		iotcon_provisioning_devices_foreach_cb cb, void *user_data)
{
	int ret;
	OCProvisionDev_t *current;
	iotcon_provisioning_device_h device;

	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	current = devices->dev_list;

	for (; current; current = current->next) {
		ret = _provisioning_device_create(current, &device);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_provisioning_device_create() Fail(%d)", ret);
			return ret;
		}

		if (IOTCON_FUNC_STOP == cb(devices, device, user_data)) {
			iotcon_provisioning_device_destroy(device);
			break;
		}
		iotcon_provisioning_device_destroy(device);
	}

	return IOTCON_ERROR_NONE;
}


OCProvisionDev_t* icl_provisioning_devices_get_devices(
		iotcon_provisioning_devices_h devices)
{
	FN_CALL;
	RETV_IF(NULL == devices, NULL);

	icl_provisioning_devices_print_uuid(devices);

	return devices->dev_list;
}

/*
API int iotcon_provisioning_devices_get_count(iotcon_provisioning_devices_h devices,
		int *count)
{
	int i;
	OCProvisionDev_t *device_list = devices->dev_list;

	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);

	if (NULL == devices) {
		DBG("Not Found");
		return IOTCON_ERROR_NO_DATA;
	}

	for (i = 0; device_list; i++)
		device_list = device_list->next;

	*count = i;

	return IOTCON_ERROR_NONE;
}
*/

static bool _provisioning_compare_oic_uuid(OicUuid_t *a, OicUuid_t *b)
{
	int i;
	bool ret = true;

	for (i = 0; i < UUID_LENGTH; i++) {
		if ((*a).id[i] != (*b).id[i]) {
			ret = false;
			break;
		}
	}
	return ret;
}


int icl_provisioning_devices_move_device(OicUuid_t *a,
		iotcon_provisioning_devices_h unowned_devices,
		iotcon_provisioning_devices_h owned_devices)
{
	FN_CALL;
	OCProvisionDev_t *owned_dev_list = owned_devices->dev_list;
	OCProvisionDev_t *previous;
	OCProvisionDev_t *current;

	previous = NULL;
	current = icl_provisioning_devices_get_devices(unowned_devices);

	for (; current; previous = current, current = current->next) {
		if (false == _provisioning_compare_oic_uuid(a, &current->doxm->deviceID))
			continue;

		if (previous)
			previous->next = current->next;
		else
			unowned_devices->dev_list = current->next;

		current->next = owned_dev_list;
		owned_devices->dev_list = current;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_devices_add_device(iotcon_provisioning_devices_h devices,
		iotcon_provisioning_device_h device)
{
	FN_CALL;
	OCProvisionDev_t *previous = NULL;
	OCProvisionDev_t *current;
	OCProvisionDev_t *dev_list = devices->dev_list;

	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);

	current = PMCloneOCProvisionDev(device->device);
	if (NULL == current) {
		ERR("PMCloneOCProvisionDev() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	for (; dev_list;) {
		previous = dev_list;
		dev_list = dev_list->next;
	}
	previous->next = current;

	return IOTCON_ERROR_NONE;
}


void icl_provisioning_device_print_uuid(iotcon_provisioning_device_h device)
{
	if (NULL == device)
		return;

	DBG("Device ID : %s", device->device_id);
}


static void _provisioning_print_uuid(OicUuid_t *uuid)
{
	DBG("Device ID : %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			(*uuid).id[0], (*uuid).id[1], (*uuid).id[2], (*uuid).id[3],
			(*uuid).id[4], (*uuid).id[5], (*uuid).id[6], (*uuid).id[7],
			(*uuid).id[8], (*uuid).id[9], (*uuid).id[10], (*uuid).id[11],
			(*uuid).id[12], (*uuid).id[13], (*uuid).id[14], (*uuid).id[15]);
}


void icl_provisioning_devices_print_uuid(iotcon_provisioning_devices_h devices)
{
	OCProvisionDev_t *dev_list;

	if (NULL == devices)
		return;

	for (dev_list = devices->dev_list; dev_list; dev_list = dev_list->next)
		_provisioning_print_uuid(&dev_list->doxm->deviceID);
}


API int iotcon_provisioning_acl_create(iotcon_provisioning_acl_h *acl)
{
	iotcon_provisioning_acl_h temp;

	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);

	temp = calloc(1, sizeof(struct icl_provisioning_acl));
	if (NULL == temp) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	*acl = temp;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_acl_set_subject(iotcon_provisioning_acl_h acl,
		iotcon_provisioning_device_h device)
{
	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);

	acl->device = PMCloneOCProvisionDev(device->device);

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_acl_add_resource(iotcon_provisioning_acl_h acl,
		const char *uri_path, iotcon_provisioning_permission_e permission)
{
	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	acl->resource_list = g_list_append(acl->resource_list, ic_utils_strdup(uri_path));
	acl->permission = permission;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_acl_destroy(iotcon_provisioning_acl_h acl)
{
	RETV_IF(false == ic_utils_check_ocf_security_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);

	OCDeleteDiscoveredDevices(acl->device);

	free(acl);

	return IOTCON_ERROR_NONE;
}
