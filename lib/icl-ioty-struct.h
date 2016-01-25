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

#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_STRUCT_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_STRUCT_H__

#include "iotcon-types.h"
#include "icl-ioty.h"

int icl_ioty_parse_find_payload(OCDevAddr *dev_addr, OCDiscoveryPayload *discovered,
		iotcon_remote_resource_h **resource_list, int *resource_count);
int icl_ioty_parse_device_info_payload(OCDevicePayload *payload,
		iotcon_device_info_h *device_info);
int icl_ioty_parse_platform_info_payload(OCPlatformPayload *payload,
		iotcon_platform_info_h *platform_info);
int icl_ioty_parse_presence(OCDevAddr *dev_addr, OCPresencePayload *payload,
		OCStackResult result, iotcon_presence_response_h *presence_response);
int icl_ioty_parse_representation_payload(OCRepPayload *payload, bool is_parent,
		iotcon_representation_h *representation);

#endif /*__IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_STRUCT_H__*/
