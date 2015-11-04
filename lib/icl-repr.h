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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_H__

#include <glib.h>
#include <tizen_type.h>

#include "iotcon-struct.h"
#include "icl-repr-value.h"

struct icl_representation_s {
	char *uri_path;
	int ref_count;
	int interfaces;
	int visibility;
	GList *children;
	iotcon_resource_types_h res_types;
	struct icl_state_s *state;
};

struct icl_state_s {
	int ref_count;
	GHashTable *hash_table;
};

void icl_representation_inc_ref_count(iotcon_representation_h val);

void icl_state_clone_foreach(char *key, iotcon_value_h src_val, iotcon_state_h dest_state);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_H__ */
