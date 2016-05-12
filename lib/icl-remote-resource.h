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
#ifndef __IOT_CONNECTIVITY_LIBRARY_CLIENT_H__
#define __IOT_CONNECTIVITY_LIBRARY_CLIENT_H__

#include <stdint.h>
#include <gio/gio.h>

#include "iotcon-types.h"
#include "iotcon-remote-resource.h"
#include "icl-options.h"

struct icl_remote_resource {
	bool is_found;
	int ref_count;
	char *uri_path;
	char *host_address;
	char *device_id;
	char *device_name;
	int properties;
	iotcon_options_h header_options;
	iotcon_resource_types_h types;
	iotcon_resource_interfaces_h ifaces;
	iotcon_connectivity_type_e connectivity_type;
	int64_t observe_handle;
	iotcon_representation_h cached_repr;
};

void icl_remote_resource_ref(iotcon_remote_resource_h resource);
void icl_remote_resource_unref(iotcon_remote_resource_h resource);

#endif /* __IOT_CONNECTIVITY_LIBRARY_CLIENT_H__ */
