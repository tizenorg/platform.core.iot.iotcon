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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_VALUE_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_VALUE_H__

#include <json-glib/json-glib.h>

#include "iotcon-struct.h"

enum {
	/* SHOULD NOT change this order */
	IOTCON_VALUE_NONE = 0,
	IOTCON_VALUE_INT,
	IOTCON_VALUE_BOOL,
	IOTCON_VALUE_DOUBLE,
	IOTCON_VALUE_STR,
	IOTCON_VALUE_NULL,
	IOTCON_VALUE_LIST,
	IOTCON_VALUE_REPR,
};

iotcon_value_h ic_value_new(int type);

int ic_value_set_int(iotcon_value_h value, int ival);
int ic_value_set_bool(iotcon_value_h value, bool bval);
int ic_value_set_double(iotcon_value_h value, double dbval);
int ic_value_set_str(iotcon_value_h value, const char *strval);
int ic_value_set_list(iotcon_value_h value, iotcon_list_h list);
int ic_value_set_rep(iotcon_value_h value, iotcon_repr_h rep);
int ic_value_set_null(iotcon_value_h value);

JsonNode* ic_repr_generate_json_value(iotcon_value_h value);
iotcon_value_h ic_repr_parse_json_value(JsonNode *node);

void ic_repr_free_value(gpointer data);
void ic_repr_free_basic_value(iotcon_value_h iotvalue);

#endif // __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_VALUE_H__
