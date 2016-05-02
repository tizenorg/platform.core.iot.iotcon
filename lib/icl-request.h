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
#ifndef __IOT_CONNECTIVITY_LIBRARY_REQUEST_H__
#define __IOT_CONNECTIVITY_LIBRARY_REQUEST_H__

#include <stdint.h>
#include "iotcon-types.h"

struct icl_observe_info {
	iotcon_observe_type_e action;
	int observe_id;
};

struct icl_resource_request {
	char *host_address;
	iotcon_connectivity_type_e connectivity_type;
	iotcon_request_type_e type;
	iotcon_options_h header_options;
	iotcon_query_h query;
	struct icl_observe_info observation_info;
	iotcon_representation_h repr;
	int64_t oic_request_h;
	int64_t oic_resource_h;
};

#endif /* __IOT_CONNECTIVITY_LIBRARY_REQUEST_H__ */
