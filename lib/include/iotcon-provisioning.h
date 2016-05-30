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
#ifndef __IOT_CONNECTIVITY_PROVISIONING_H__
#define __IOT_CONNECTIVITY_PROVISIONING_H__

#ifdef __cplusplus
extern "C" {
#endif

// -> iotcon-types.h
typedef struct icl_provisioning_devices* iotcon_provisioning_devices_h;
typedef struct icl_provisioning_device* iotcon_provisioning_device_h;
typedef struct icl_provisioning_acl* iotcon_provisioning_acl_h;

// -> iotcon-constant.h
typedef enum {
	IOTCON_PERMISSION_CREATE,
	IOTCON_PERMISSION_READ,
	IOTCON_PERMISSION_WRITE,
	IOTCON_PERMISSION_DELETE,
	IOTCON_PERMISSION_NOTIFY,
	IOTCON_PERMISSION_FULL_CONTROL,
} iotcon_provisioning_permission_e;

/* initialize */
int iotcon_provisioning_initialize(const char *file_path, const char *db_path);

typedef char* (*iotcon_provisioning_randompins_cb)(int len, void *user_data);

int iotcon_provisioning_set_randompins(iotcon_provisioning_randompins_cb cb,
		void *user_data);

/* discover */
int iotcon_provisioning_get_devices(iotcon_provisioning_devices_h *owned_devices,
		iotcon_provisioning_devices_h *unowned_devices);

typedef void (*iotcon_provisioning_found_devices_cb)(
		iotcon_provisioning_devices_h owned_devices,
		iotcon_provisioning_devices_h unowned_devices,
		void *user_data);

int iotcon_provisioning_find_all_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data);

int iotcon_provisioning_find_unowned_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data);

int iotcon_provisioning_find_owned_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data);

/* register */
typedef void (*iotcon_provisioning_ownership_transfer_cb)(
		iotcon_provisioning_devices_h owned_devices,
		iotcon_provisioning_devices_h unowned_devices,
		void *user_data);

int iotcon_provisioning_register_unowned_devices(
		iotcon_provisioning_devices_h devices,
		iotcon_provisioning_ownership_transfer_cb cb,
		void *user_data);

/* cred */
typedef void (*iotcon_provisioning_provision_cred_cb)(
		iotcon_provisioning_device_h device1,
		iotcon_provisioning_device_h device2,
		void *user_data);

int iotcon_provisioning_provision_cred(iotcon_provisioning_device_h device1,
		iotcon_provisioning_device_h device2,
		iotcon_provisioning_provision_cred_cb cb,
		void *user_data);

/* acl */
typedef void (*iotcon_provisioning_acl_cb)(void *user_data);

int iotcon_provisioning_acl(iotcon_provisioning_device_h device,
		iotcon_provisioning_acl_h acl,
		iotcon_provisioning_acl_cb cb,
		void *user_data);

/* struct */
int iotcon_provisioning_device_clone(iotcon_provisioning_device_h device,
		iotcon_provisioning_device_h *cloned_device);
void iotcon_provisioning_device_destroy(iotcon_provisioning_device_h device);

int iotcon_provisioning_devices_create(iotcon_provisioning_devices_h *devices);
int iotcon_provisioning_devices_destroy(iotcon_provisioning_devices_h devices);
int iotcon_provisioning_devices_clone(iotcon_provisioning_devices_h devices,
		iotcon_provisioning_devices_h *cloned_devices);

typedef bool (*iotcon_provisioning_devices_foreach_cb)(
		iotcon_provisioning_devices_h devices,
		iotcon_provisioning_device_h device,
		void *user_data);
int iotcon_provisioning_devices_foreach(iotcon_provisioning_devices_h devices,
		iotcon_provisioning_devices_foreach_cb cb, void *user_data);

int iotcon_provisioning_devices_add_device(iotcon_provisioning_devices_h devices,
		iotcon_provisioning_device_h device);

int iotcon_provisioning_acl_create(iotcon_provisioning_acl_h *acl);
int iotcon_provisioning_acl_set_subject(iotcon_provisioning_acl_h acl,
		iotcon_provisioning_device_h);
int iotcon_provisioning_acl_add_resource(iotcon_provisioning_acl_h acl,
		const char *uri_path, iotcon_provisioning_permission_e permission);
int iotcon_provisioning_acl_destroy(iotcon_provisioning_acl_h acl);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_PROVISIONING_H__ */
