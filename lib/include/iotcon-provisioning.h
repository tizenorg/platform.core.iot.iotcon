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
typedef struct icl_provisioning_acl_s* iotcon_provisioning_acl_h;

int iotcon_provisioning_init(const char *file_path, const char *db_path);

typedef void (*iotcon_provisioning_randompins_cb)(char *pin, int len, void *user_data);

int iotcon_provisioning_set_randompins(iotcon_provisioning_randompins_cb cb,
		void *user_data);

typedef void (*iotcon_provisioning_found_devices_cb)(
		iotcon_provisioning_devices_h owned_devices,
		iotcon_provisioning_devices_h unowned_devices,
		void *user_data);

int iotcon_provisioning_discover_all_devices(int timeout,
		iotcon_provisioning_found_devices_cb cb, void *user_data);

typedef void (*iotcon_provisioning_ownership_transfer_cb)(
		iotcon_provisioning_devices_h owned_devices,
		iotcon_provisioning_devices_h unowned_devices,
		void *user_data);

int iotcon_provisioning_register_unowned_devices(
		iotcon_provisioning_devices_h owned_devices,
		iotcon_provisioning_devices_h unowned_devices,
		iotcon_provisioning_ownership_transfer_cb cb,
		void *user_data);

int iotcon_provisioning_devices_create(iotcon_provisioning_devices_h *devices);
int iotcon_provisioning_devices_destroy(iotcon_provisioning_devices_h devices);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_PROVISIONING_H__ */
