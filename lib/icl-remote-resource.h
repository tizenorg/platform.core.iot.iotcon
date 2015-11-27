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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_CLIENT_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_CLIENT_H__

#include <stdint.h>
#include <gio/gio.h>

#include "iotcon-types.h"
#include "iotcon-remote-resource.h"

#include "icl-options.h"

typedef enum {
	ICL_DEVICE_STATE_ALIVE,
	ICL_DEVICE_STATE_LOST_SIGNAL,
} icl_remote_resource_device_state_e;

struct icl_remote_resource {
	char *uri_path;
	char *host_address;
	char *device_id;
	int properties;
	iotcon_options_h header_options;
	iotcon_resource_types_h types;
	int ifaces;
	iotcon_connectivity_type_e connectivity_type;
	int64_t observe_handle;
	unsigned int observe_sub_id;
	unsigned int monitoring_sub_id;
	unsigned int caching_sub_id;
	iotcon_representation_h cached_repr;
};

void icl_remote_resource_crud_stop(iotcon_remote_resource_h resource);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_CLIENT_H__ */
