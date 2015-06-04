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

#include <glib.h>
#include <json-glib/json-glib.h>

#include "iotcon-struct.h"

struct ic_list_s {
	int type;
	int ref_count;
	GList *list;
};

int ic_list_remove(iotcon_list_h list, iotcon_value_h val);
int ic_list_insert(iotcon_list_h list, iotcon_value_h value, int pos);

JsonArray* ic_list_to_json(iotcon_list_h list);
iotcon_list_h ic_list_from_json(JsonArray *parray);

iotcon_list_h ic_list_clone(iotcon_list_h list);

void ic_list_inc_ref_count(iotcon_list_h val);

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_LIST_H__ */
