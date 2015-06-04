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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_TYPES_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_TYPES_H__

#include "iotcon-struct.h"

struct ic_resource_types {
	int ref_count;
	GList *type_list;
};

iotcon_resource_types_h ic_resource_types_ref(iotcon_resource_types_h res_types);
const char* ic_resource_types_get_nth_data(iotcon_resource_types_h res_types, int index);
unsigned int ic_resource_types_get_length(iotcon_resource_types_h res_types);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_TYPES_H__ */
