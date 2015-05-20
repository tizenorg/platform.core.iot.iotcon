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
#ifndef __IOT_CONNECTIVITY_MANAGER_REPRESENTATION_H__
#define __IOT_CONNECTIVITY_MANAGER_REPRESENTATION_H__

iotcon_repr_h iotcon_repr_new();
void iotcon_repr_free(iotcon_repr_h repr);
iotcon_repr_h iotcon_repr_clone(const iotcon_repr_h src);

int iotcon_repr_set_uri(iotcon_repr_h repr, const char *uri);
const char* iotcon_repr_get_uri(iotcon_repr_h repr);
int iotcon_repr_delete_uri(iotcon_repr_h repr);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Appends resource type name.
 * @since_tizen 3.0
 * @remarks  Duplicate type names are allowed to append.
 *
 * @param[in] repr The handle to the Representation
 * @param[in] type The resource type
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PARAM  Invalid parameter
 */
int iotcon_repr_append_resource_types(iotcon_repr_h repr, const char *type);
typedef void (*iotcon_resourcetype_fn)(const char *res_type, void *user_data);
void iotcon_repr_get_resource_types(iotcon_repr_h repr, iotcon_resourcetype_fn fn,
		void *user_data);
int iotcon_repr_get_resource_types_count(iotcon_repr_h repr);
int iotcon_repr_delete_resource_types(iotcon_repr_h repr, const char *type);

int iotcon_repr_append_resource_interfaces(iotcon_repr_h repr, const char *interface);
typedef void (*iotcon_interface_fn)(const char *res_if, void *user_data);
void iotcon_repr_get_resource_interfaces(iotcon_repr_h repr, iotcon_interface_fn fn,
		void *user_data);
int iotcon_repr_get_resource_interfaces_count(iotcon_repr_h repr);
int iotcon_repr_delete_resource_interfaces(iotcon_repr_h repr, const char *type);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Sets int value.
 * @since_tizen 3.0
 * @remarks Stored value is replaced with @a ival.
 *
 * @param[in] repr The handle to the Representation
 * @param[in] type The resource type
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PARAM  Invalid parameter
 */
int iotcon_repr_set_int(iotcon_repr_h repr, const char *key, int val);
int iotcon_repr_set_bool(iotcon_repr_h repr, const char *key, bool val);
int iotcon_repr_set_double(iotcon_repr_h repr, const char *key, double val);
int iotcon_repr_set_str(iotcon_repr_h repr, const char *key, char *val);
int iotcon_repr_set_list(iotcon_repr_h repr, const char *key, iotcon_list_h list);
int iotcon_repr_set_repr(iotcon_repr_h dest, const char *key, iotcon_repr_h src);
int iotcon_repr_set_null(iotcon_repr_h repr, const char *key);

int iotcon_repr_get_int(iotcon_repr_h repr, const char *key);
bool iotcon_repr_get_bool(iotcon_repr_h repr, const char *key);
double iotcon_repr_get_double(iotcon_repr_h repr, const char *key);
char* iotcon_repr_get_str(iotcon_repr_h repr, const char *key);
iotcon_list_h iotcon_repr_get_list(iotcon_repr_h repr, const char *key);
iotcon_repr_h iotcon_repr_get_repr(iotcon_repr_h repr, const char *key);
bool iotcon_repr_is_null(iotcon_repr_h repr, const char *key);

int iotcon_repr_del_int(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_bool(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_double(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_str(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_list(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_repr(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_null(iotcon_repr_h repr, const char *key);

int iotcon_repr_append_child(iotcon_repr_h parent, iotcon_repr_h child);
typedef void (*iotcon_children_fn)(iotcon_repr_h child, void *user_data);
void iotcon_repr_get_children(iotcon_repr_h parent, iotcon_children_fn fn,
		void *user_data);
int iotcon_repr_get_children_count(iotcon_repr_h parent);
iotcon_repr_h iotcon_repr_get_nth_child(iotcon_repr_h parent, int index);

GList* iotcon_repr_get_key_list(iotcon_repr_h repr);
int iotcon_repr_get_keys_count(iotcon_repr_h repr);

char* iotcon_repr_generate_json(iotcon_repr_h repr);

iotcon_list_h iotcon_list_new(iotcon_repr_types_e type);
void iotcon_list_free(iotcon_list_h list);

iotcon_list_h iotcon_list_insert_int(iotcon_list_h list, int val, int pos);
iotcon_list_h iotcon_list_insert_bool(iotcon_list_h list, bool val, int pos);
iotcon_list_h iotcon_list_insert_double(iotcon_list_h list, double val, int pos);
iotcon_list_h iotcon_list_insert_str(iotcon_list_h list, char *val, int pos);
iotcon_list_h iotcon_list_insert_list(iotcon_list_h list, iotcon_list_h val, int pos);
iotcon_list_h iotcon_list_insert_repr(iotcon_list_h list, iotcon_repr_h val, int pos);

int iotcon_list_get_nth_int(iotcon_list_h list, int index);
bool iotcon_list_get_nth_bool(iotcon_list_h list, int index);
double iotcon_list_get_nth_double(iotcon_list_h list, int index);
const char* iotcon_list_get_nth_str(iotcon_list_h list, int index);
iotcon_list_h iotcon_list_get_nth_list(iotcon_list_h list, int index);
iotcon_repr_h iotcon_list_get_nth_repr(iotcon_list_h list, int index);

int iotcon_list_del_nth_int(iotcon_list_h list, int pos);
int iotcon_list_del_nth_bool(iotcon_list_h list, int pos);
int iotcon_list_del_nth_double(iotcon_list_h list, int pos);
int iotcon_list_del_nth_str(iotcon_list_h list, int pos);
int iotcon_list_del_nth_list(iotcon_list_h list, int pos);
int iotcon_list_del_nth_repr(iotcon_list_h list, int pos);

int iotcon_list_get_type(iotcon_list_h list);
int iotcon_list_get_length(iotcon_list_h list);

typedef void (*iotcon_int_list_fn)(int index, const int value, void *user_data);
void iotcon_int_list_foreach(iotcon_list_h list, iotcon_int_list_fn fn, void *user_data);
typedef void (*iotcon_bool_list_fn)(int index, const bool value, void *user_data);
void iotcon_bool_list_foreach(iotcon_list_h list, iotcon_bool_list_fn fn, void *user_data);
typedef void (*iotcon_double_list_fn)(int index, const double value, void *user_data);
void iotcon_double_list_foreach(iotcon_list_h list, iotcon_double_list_fn fn, void *user_data);
typedef void (*iotcon_str_list_fn)(int index, const char *value, void *user_data);
void iotcon_str_list_foreach(iotcon_list_h list, iotcon_str_list_fn fn, void *user_data);
typedef void (*iotcon_list_list_fn)(int index, iotcon_list_h value, void *user_data);
void iotcon_list_list_foreach(iotcon_list_h list, iotcon_list_list_fn fn, void *user_data);
typedef void (*iotcon_repr_list_fn)(int index, iotcon_repr_h value, void *user_data);
void iotcon_repr_list_foreach(iotcon_list_h list, iotcon_repr_list_fn fn, void *user_data);

#endif /* __IOT_CONNECTIVITY_MANAGER_REPRESENTATION_H__ */
