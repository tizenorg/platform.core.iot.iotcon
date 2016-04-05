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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_PRESENCE_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_PRESENCE_H__

#include <octypes.h>

typedef struct icl_presence {
	char *host_address;
	iotcon_connectivity_type_e connectivity_type;
	char *resource_type;
	iotcon_presence_cb cb;
	void *user_data;
	OCDoHandle handle;
} icl_presence_s;

typedef struct icl_presence_response {
	char *host_address;
	iotcon_connectivity_type_e connectivity_type;
	char *resource_type;
	iotcon_presence_result_e result;
	iotcon_presence_trigger_e trigger;
} icl_presence_response_s;

#endif /*__IOT_CONNECTIVITY_MANAGER_LIBRARY_PRESENCE_H__*/
