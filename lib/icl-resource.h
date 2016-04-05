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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_H__

#include <stdint.h>

#include "iotcon-types.h"

#include "icl-dbus.h"

/**
 * @brief The maximum number of children resources which can be held in a parent resource.
 *
 * @since_tizen 3.0
 */
#define ICL_CONTAINED_RESOURCES_MAX 5

/**
 * @brief The maximum length of uri_path which can be held in a resource.
 *
 * @since_tizen 3.0
 */
#define ICL_URI_PATH_LENGTH_MAX 36

struct icl_notify_msg {
	char *iface;
	iotcon_representation_h repr;
};

struct icl_resource {
	char *uri_path;
	int properties;
	iotcon_resource_types_h types;
	iotcon_resource_interfaces_h ifaces;
	iotcon_request_handler_cb cb;
	void *user_data;
	unsigned int sub_id;
	int64_t handle;
	iotcon_resource_h children[ICL_CONTAINED_RESOURCES_MAX];
	iotcon_observers_h observers;
	iotcon_connectivity_type_e connectivity_type;
};

#endif /*__IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_H__*/
