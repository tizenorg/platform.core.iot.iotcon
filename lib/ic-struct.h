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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_STRUCT_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_STRUCT_H__

#include "iotcon-struct.h"

struct ic_value_s {
	int type;
};

struct ic_list_s {
	int type;
	GList *list;
};

struct ic_repr_s {
	char *uri;
	GHashTable *hash_table;
	GList *children;
	GList *res_types;
	GList *interfaces;
};

typedef struct {
	int type;
	union {
		int i;
		bool b;
		double d;
		char *s;
	} val;
} ic_basic_s;

typedef struct {
	int type;
	struct ic_list_s *list;
} ic_val_list_s;

typedef struct {
	int type;
	struct ic_repr_s *repr;
} ic_val_repr_s;

struct ic_options_s {
	bool has_parent;
	GHashTable *options;
};

/* related with iotcon_response_property_e */
struct ic_res_response_s {
	char *new_resource_uri;
	int error_code;
	iotcon_options_h header_options;
	iotcon_interface_e interface;
	iotcon_request_h request_handle;
	iotcon_resource_h resource_handle;
	iotcon_entity_handler_result_e result;
	iotcon_repr_h repr;
};

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_STRUCT_H__ */
