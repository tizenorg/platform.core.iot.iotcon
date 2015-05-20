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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_OBJECT_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_OBJECT_H__

#include <json-glib/json-glib.h>

int ic_obj_del_value(iotcon_repr_h repr, const char *key,
		iotcon_repr_types_e value_type);

int ic_repr_obj_get_value(iotcon_repr_h repr, const char *key, iotcon_value_h *retval);
int ic_obj_set_value(iotcon_repr_h repr, const char *key, iotcon_value_h value);

JsonObject* ic_obj_to_json(iotcon_repr_h repr);
iotcon_repr_h ic_obj_from_json(JsonObject *obj);

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_REPRESENTATION_OBJECT_H__ */
