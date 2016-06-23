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

struct icl_provisioning_device {
	bool is_found;
	OCProvisionDev_t *device;
	char *host_address;
	int connectivity_type;
	char *device_id;
};

struct icl_provisioning_devices {
	bool is_found;
	OCProvisionDev_t *dev_list;
};

struct icl_provisioning_acl {
	OCProvisionDev_t *device;
	GList *resource_list;
	int permission;
};


static char* _provisioning_parse_uuid(OicUuid_t *uuid)
{
	char uuid_string[256] = {0};

	snprintf(uuid_string, sizeof(uuid_string),
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			(*uuid).id[0], (*uuid).id[1], (*uuid).id[2], (*uuid).id[3],
			(*uuid).id[4], (*uuid).id[5], (*uuid).id[6], (*uuid).id[7],
			(*uuid).id[8], (*uuid).id[9], (*uuid).id[10], (*uuid).id[11],
			(*uuid).id[12], (*uuid).id[13], (*uuid).id[14], (*uuid).id[15]);

	return strdup(uuid_string);
}


static OCProvisionDev_t* _provisioning_device_clone(OCProvisionDev_t *src)
{
	FN_CALL;

	OCProvisionDev_t *clone;

	RETV_IF(NULL == src, NULL);

	clone = PMCloneOCProvisionDev(src);
	if (NULL == clone) {
		ERR("PMCloneOCProvisionDev() Fail");
		return NULL;
	}

	if (clone->pstat) {
		if (src->pstat->sm) {
			clone->pstat->sm = calloc(1, sizeof(OicSecDpom_t));
			if (NULL == clone->pstat->sm) {
				ERR("calloc() Fail(%d)", errno);
				return NULL;
			}
			memcpy(clone->pstat->sm, src->pstat->sm, sizeof(OicSecDpom_t));
		}
	}

	if (clone->doxm) {
		if (src->doxm->oxmType) {
			clone->doxm->oxmType = calloc(1, sizeof(OicUrn_t));
			if (NULL == clone->doxm->oxmType) {
				ERR("calloc() Fail(%d)", errno);
				return NULL;
			}
			memcpy(clone->doxm->oxmType, src->doxm->oxmType, sizeof(OicUrn_t));
		}
		if (src->doxm->oxm) {
			clone->doxm->oxm = calloc(1, sizeof(OicSecOxm_t));
			if (NULL == clone->doxm->oxm) {
				ERR("calloc() Fail(%d)", errno);
				return NULL;
			}
			memcpy(clone->doxm->oxm, src->doxm->oxm, sizeof(OicSecOxm_t));
		}
	}

	return clone;
}

/*
void icl_provisioning_device_destroy(OCProvisionDev_t *src)
{
	FN_CALL;

	OCDeleteDiscoveredDevices(src);
}
*/


static int _provisioning_device_get_host_address(OCProvisionDev_t *device,
		char **host_address, int *connectivity_type)
{
	FN_CALL;
	char host[PATH_MAX] = {0};
	int temp_connectivity_type = CT_DEFAULT;

	if (device->connType & CT_ADAPTER_IP) {
		if (device->connType & CT_IP_USE_V4) {
			temp_connectivity_type = IOTCON_CONNECTIVITY_IPV4;
		} else if (device->connType & CT_IP_USE_V6) {
			temp_connectivity_type = IOTCON_CONNECTIVITY_IPV6;
		} else {
			ERR("Invalid Connectivity Type(%d)", device->connType);
			return IOTCON_ERROR_IOTIVITY;
		}
	} else {
		ERR("Invalid Connectivity Type(%d)", device->connType);
		return IOTCON_ERROR_IOTIVITY;
	}

	switch (temp_connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV6:
		snprintf(host, sizeof(host), "[%s]:%d", device->endpoint.addr,
				device->endpoint.port);
		break;
	case IOTCON_CONNECTIVITY_IPV4:
		snprintf(host, sizeof(host), "%s:%d", device->endpoint.addr,
				device->endpoint.port);
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", device->connType);
		return IOTCON_ERROR_IOTIVITY;
	}

	*host_address = strdup(host);
	*connectivity_type = temp_connectivity_type;

	return IOTCON_ERROR_NONE;
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

	temp->device = _provisioning_device_clone(device);
	if (NULL == temp->device) {
		ERR("_provisioning_device_clone() Fail");
		free(temp);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = _provisioning_device_get_host_address(device, &temp->host_address,
			&temp->connectivity_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_device_get_host_address() Fail(%d)", ret);
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

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
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


API int iotcon_provisioning_device_destroy(iotcon_provisioning_device_h device)
{
	FN_CALL;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);

	if (true == device->is_found) {
		ERR("It can't be destroyed by user.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	free(device->host_address);
	free(device->device_id);

	OCDeleteDiscoveredDevices(device->device);

	free(device);

	return IOTCON_ERROR_NONE;
}


void icl_provisioning_device_set_found(iotcon_provisioning_device_h device)
{
	if (NULL == device)
		return;

	device->is_found = true;
}


void icl_provisioning_device_unset_found(iotcon_provisioning_device_h device)
{
	if (NULL == device)
		return;

	device->is_found = false;
}


OCProvisionDev_t* icl_provisioning_device_get_device(
		iotcon_provisioning_device_h device)
{
	FN_CALL;
	RETV_IF(NULL == device, NULL);

	return device->device;
}


API int iotcon_provisioning_device_get_host_address(iotcon_provisioning_device_h device,
		char **host_address)
{
	FN_CALL;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);

	*host_address = device->host_address;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_device_get_connectivity_type(
		iotcon_provisioning_device_h device,
		iotcon_connectivity_type_e *connectivity_type)
{
	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == connectivity_type, IOTCON_ERROR_INVALID_PARAMETER);

	*connectivity_type = device->connectivity_type;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_device_get_id(iotcon_provisioning_device_h device,
		char **device_id)
{
	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device_id, IOTCON_ERROR_INVALID_PARAMETER);

	*device_id = device->device_id;

	return IOTCON_ERROR_NONE;
}


static int _provisioning_parse_oxm(OicSecOxm_t oic_oxm, iotcon_provisioning_oxm_e *oxm)
{
	iotcon_provisioning_oxm_e temp;

	switch (oic_oxm) {
	case OIC_JUST_WORKS:
		temp = IOTCON_OXM_JUST_WORKS;
		break;
	case OIC_RANDOM_DEVICE_PIN:
		temp = IOTCON_OXM_RANDOM_PIN;
		break;
	case OIC_MANUFACTURER_CERTIFICATE:
	default:
		ERR("Invalid oxm(%d)", oic_oxm);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*oxm = temp;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_device_get_oxm(iotcon_provisioning_device_h device,
		iotcon_provisioning_oxm_e *oxm)
{
	int ret;
	iotcon_provisioning_oxm_e temp;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == oxm, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device->device, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device->device->doxm, IOTCON_ERROR_INVALID_PARAMETER);

	ret = _provisioning_parse_oxm(device->device->doxm->oxmSel, &temp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_parse_oxm() Fail(%d)", ret);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*oxm = temp;

	return IOTCON_ERROR_NONE;
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

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
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
	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);

	if (true == devices->is_found) {
		ERR("It can't be destroyed by user.");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if (devices->dev_list)
		OCDeleteDiscoveredDevices(devices->dev_list);

	free(devices);

	return IOTCON_ERROR_NONE;
}


OCProvisionDev_t* icl_provisioning_devices_clone(OCProvisionDev_t *src)
{
	OCProvisionDev_t *clone;
	OCProvisionDev_t *current;

	if (NULL == src)
		return NULL;

	clone = _provisioning_device_clone(src);
	if (NULL == clone) {
		ERR("_provisioning_device_clone() Fail");
		return NULL;
	}

	current = clone;
	src = src->next;

	for (; src; src = src->next, current = current->next) {
		current->next = _provisioning_device_clone(src);
		if (NULL == current->next) {
			ERR("_provisioning_device_clone() Fail");
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

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == devices->dev_list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cloned_devices, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_provisioning_devices_create(&temp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_devices_create() Fail(%d)", ret);
		return ret;
	}

	temp->dev_list = icl_provisioning_devices_clone(devices->dev_list);
	if (NULL == temp->dev_list) {
		ERR("icl_provisioning_devices_clone() Fail");
		free(temp);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	*cloned_devices = temp;

	return IOTCON_ERROR_NONE;
}


void icl_provisioning_devices_set_found(iotcon_provisioning_devices_h devices)
{
	if (NULL == devices)
		return;

	devices->is_found = true;
}


void icl_provisioning_devices_unset_found(iotcon_provisioning_devices_h devices)
{
	if (NULL == devices)
		return;

	devices->is_found = false;
}


API int iotcon_provisioning_devices_foreach(iotcon_provisioning_devices_h devices,
		iotcon_provisioning_devices_foreach_cb cb, void *user_data)
{
	int ret;
	OCProvisionDev_t *current;
	iotcon_provisioning_device_h device;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
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


bool icl_provisioning_compare_oic_uuid(OicUuid_t *a, OicUuid_t *b)
{
	int i;
	bool ret = true;

	RETV_IF(NULL == a, false);
	RETV_IF(NULL == b, false);

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
		if (false == icl_provisioning_compare_oic_uuid(a, &current->doxm->deviceID))
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
	OCProvisionDev_t *current;
	OCProvisionDev_t *dev_list;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == devices, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);

	dev_list = devices->dev_list;

	current = icl_provisioning_devices_get_devices(devices);
	for (; current; current = current->next) {
		if (true == icl_provisioning_compare_oic_uuid(&current->doxm->deviceID,
					&device->device->doxm->deviceID)) {
			ERR("%s is already contained.", device->device_id);
			return IOTCON_ERROR_ALREADY;
		}
	}

	current = _provisioning_device_clone(device->device);
	if (NULL == current) {
		ERR("_provisioning_device_clone() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	if (NULL == dev_list)
		dev_list = current;
	else {
		while (dev_list->next)
			dev_list = dev_list->next;
		dev_list->next = current;
	}

	return IOTCON_ERROR_NONE;
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

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);

	temp = calloc(1, sizeof(struct icl_provisioning_acl));
	if (NULL == temp) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	*acl = temp;

	return IOTCON_ERROR_NONE;
}


static gpointer _provisioning_acl_resource_list_clone(gconstpointer src,
		gpointer data)
{
	return ic_utils_strdup(src);
}


int icl_provisioning_acl_clone(iotcon_provisioning_acl_h acl,
		iotcon_provisioning_acl_h *cloned_acl)
{
	int ret;
	iotcon_provisioning_acl_h temp;

	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cloned_acl, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_provisioning_acl_create(&temp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_provisioning_acl_create() Fail(%d)", ret);
		return ret;
	}

	temp->device = _provisioning_device_clone(acl->device);
	if (NULL == temp->device) {
		ERR("_provisioning_device_clone() Fail");
		iotcon_provisioning_acl_destroy(temp);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	temp->resource_list = g_list_copy_deep(acl->resource_list,
			_provisioning_acl_resource_list_clone, NULL);

	temp->permission = acl->permission;

	*cloned_acl = temp;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_acl_set_subject(iotcon_provisioning_acl_h acl,
		iotcon_provisioning_device_h device)
{
	OCProvisionDev_t *dev;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device, IOTCON_ERROR_INVALID_PARAMETER);

	dev = _provisioning_device_clone(device->device);
	if (NULL == dev) {
		ERR("_provisioning_device_clone() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	OCDeleteDiscoveredDevices(acl->device);

	acl->device = dev;

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_acl_set_all_subject(iotcon_provisioning_acl_h acl)
{
	OCProvisionDev_t *dev;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);

	dev = calloc(1, sizeof(OCProvisionDev_t));
	if (NULL == dev) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	dev->doxm = calloc(1, sizeof(OicSecDoxm_t));
	if (NULL == dev->doxm) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	memcpy(&dev->doxm->deviceID, "*", 2);

	OCDeleteDiscoveredDevices(acl->device);

	acl->device = dev;

	return IOTCON_ERROR_NONE;
}


OCProvisionDev_t* icl_provisioning_acl_get_subject(iotcon_provisioning_acl_h acl)
{
	RETV_IF(NULL == acl, NULL);

	return acl->device;
}


API int iotcon_provisioning_acl_add_resource(iotcon_provisioning_acl_h acl,
		const char *uri_path)
{
	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	acl->resource_list = g_list_append(acl->resource_list, ic_utils_strdup(uri_path));

	return IOTCON_ERROR_NONE;
}


API int iotcon_provisioning_acl_set_permission(iotcon_provisioning_acl_h acl,
		int permission)
{
	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(permission <= 0 || IOTCON_PERMISSION_FULL_CONTROL < permission,
			IOTCON_ERROR_INVALID_PARAMETER);

	acl->permission = permission;

	return IOTCON_ERROR_NONE;
}


int icl_provisioning_acl_get_resource_count(iotcon_provisioning_acl_h acl)
{
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);

	return g_list_length(acl->resource_list);
}


char* icl_provisioning_acl_get_nth_resource(iotcon_provisioning_acl_h acl, int index)
{
	RETV_IF(NULL == acl, NULL);

	return g_list_nth_data(acl->resource_list, index);
}


API int iotcon_provisioning_acl_destroy(iotcon_provisioning_acl_h acl)
{
	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == acl, IOTCON_ERROR_INVALID_PARAMETER);

	OCDeleteDiscoveredDevices(acl->device);
	g_list_free_full(acl->resource_list, free);
	free(acl);

	return IOTCON_ERROR_NONE;
}


int icl_provisioning_acl_convert_permission(int permission)
{
	int oic_permission = 0;

	if (permission & IOTCON_PERMISSION_CREATE)
		oic_permission |= PERMISSION_CREATE;
	if (permission & IOTCON_PERMISSION_READ)
		oic_permission |= PERMISSION_READ;
	if (permission & IOTCON_PERMISSION_WRITE)
		oic_permission |= PERMISSION_WRITE;
	if (permission & IOTCON_PERMISSION_DELETE)
		oic_permission |= PERMISSION_DELETE;
	if (permission & IOTCON_PERMISSION_NOTIFY)
		oic_permission |= PERMISSION_NOTIFY;

	DBG("permission : %d", oic_permission);

	return oic_permission;
}


int icl_provisioning_acl_get_permission(iotcon_provisioning_acl_h acl)
{
	RETV_IF(NULL == acl, 0);

	return acl->permission;
}

