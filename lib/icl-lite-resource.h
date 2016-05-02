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
#ifndef __IOT_CONNECTIVITY_LIBRARY_LITE_RESOURCE_H__
#define __IOT_CONNECTIVITY_LIBRARY_LITE_RESOURCE_H__

#include <stdint.h>
#include "iotcon-lite-resource.h"
#include "iotcon-types.h"

struct icl_lite_resource {
	char *uri_path;
	iotcon_state_h state;
	int64_t handle;
	int properties;
	iotcon_lite_resource_post_request_cb cb;
	void *cb_data;
	iotcon_connectivity_type_e connectivity_type;
};

#endif /* __IOT_CONNECTIVITY_LIBRARY_LITE_RESOURCE_H__ */

