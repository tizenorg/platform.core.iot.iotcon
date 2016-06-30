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
#ifndef __IOT_CONNECTIVITY_LIBRARY_PROVISIONING_STRUCT_H__
#define __IOT_CONNECTIVITY_LIBRARY_PROVISIONING_STRUCT_H__

#include <ocprovisioningmanager.h>

void icl_provisioning_device_set_found(iotcon_provisioning_device_h device);
void icl_provisioning_device_unset_found(iotcon_provisioning_device_h device);
void icl_provisioning_device_set_owned(iotcon_provisioning_device_h device);

OCProvisionDev_t* icl_provisioning_device_get_device(
		iotcon_provisioning_device_h devices);

void icl_provisioning_devices_set_found(iotcon_provisioning_devices_h devices);
void icl_provisioning_devices_unset_found(iotcon_provisioning_devices_h devices);

int icl_provisioning_devices_create(OCProvisionDev_t *dev_list,
		iotcon_provisioning_devices_h *devices);

OCProvisionDev_t* icl_provisioning_devices_get_devices(
		iotcon_provisioning_devices_h devices);

void icl_provisioning_devices_move_device(OicUuid_t *a,
		iotcon_provisioning_devices_h unowned_devices,
		iotcon_provisioning_devices_h owned_devices);

OCProvisionDev_t* icl_provisioning_devices_clone(OCProvisionDev_t *src);

void icl_provisioning_device_print_uuid(iotcon_provisioning_device_h device);
void icl_provisioning_devices_print_uuid(iotcon_provisioning_devices_h devices);
bool icl_provisioning_compare_oic_uuid(OicUuid_t *a, OicUuid_t *b);

int icl_provisioning_acl_clone(iotcon_provisioning_acl_h acl,
		iotcon_provisioning_acl_h *cloned_acl);
OCProvisionDev_t* icl_provisioning_acl_get_subject(
		iotcon_provisioning_acl_h acl);
int icl_provisioning_acl_get_resource_count(iotcon_provisioning_acl_h acl);
char* icl_provisioning_acl_get_nth_resource(iotcon_provisioning_acl_h acl, int index);
int icl_provisioning_acl_convert_permission(int permission);
int icl_provisioning_acl_get_permission(iotcon_provisioning_acl_h acl);

#endif /* __IOT_CONNECTIVITY_LIBRARY_PROVISIONING_STRUCT_H__ */
