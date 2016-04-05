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

#include "iotcon-types.h"
#include "icl-value.h"

struct icl_state_s {
	int ref_count;
	GHashTable *hash_table;
};

struct icl_representation_s {
	char *uri_path;
	int ref_count;
	int visibility;
	GList *children;
	iotcon_resource_types_h res_types;
	iotcon_resource_interfaces_h interfaces;
	struct icl_state_s *state;
};

iotcon_representation_h icl_representation_ref(iotcon_representation_h repr);
int icl_representation_compare(iotcon_representation_h repr1,
		iotcon_representation_h repr2);


#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_H__ */
