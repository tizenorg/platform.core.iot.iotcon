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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_LIST_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_LIST_H__

#include <json-glib/json-glib.h>

#include "iotcon-struct.h"

iotcon_list_h ic_list_new(int type);
int ic_list_remove(iotcon_list_h list, iotcon_value_h val);
iotcon_list_h ic_list_append(iotcon_list_h list, iotcon_value_h value);

JsonArray* ic_repr_generate_json_array(iotcon_list_h list);
iotcon_list_h ic_repr_parse_json_array(JsonArray *parray);

void ic_repr_free_list(iotcon_list_h iotlist);

#endif // __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_LIST_H__
