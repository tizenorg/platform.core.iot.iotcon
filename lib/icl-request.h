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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_REQUEST_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_REQUEST_H__

#include "iotcon-struct.h"
#include "iotcon-constant.h"

typedef void* oc_request_h;
typedef void* oc_resource_h;

struct ic_observe_info {
	iotcon_observe_action_e action;
	int observer_id;
};

struct ic_resource_request {
	int types;
	char *uri;
	iotcon_options_h header_options;
	iotcon_query_h query;
	struct ic_observe_info observation_info;
	iotcon_repr_h repr;
	oc_request_h request_handle;
	oc_resource_h resource_handle;
};

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_REQUEST_H__ */
