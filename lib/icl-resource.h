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
#ifndef __IOT_CONNECTIVITY_LIBRARY_RESOURCE_H__
#define __IOT_CONNECTIVITY_LIBRARY_RESOURCE_H__

#include <stdint.h>
#include <glib.h>

#include "iotcon-types.h"

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
	int64_t handle;
	iotcon_observers_h observers;
	GList *children;
};

bool icl_resource_check_uri_path(const char *uri_path);
bool icl_resource_check_type(const char *type);
bool icl_resource_check_interface(const char *iface);

#endif /*__IOT_CONNECTIVITY_LIBRARY_RESOURCE_H__*/
