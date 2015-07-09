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

#include <stdbool.h>
#include <iotcon-constant.h>
#include <iotcon-struct.h>

iotcon_repr_h iotcon_repr_new();
void iotcon_repr_free(iotcon_repr_h repr);
iotcon_repr_h iotcon_repr_clone(const iotcon_repr_h src);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Appends resource type name.
 * @since_tizen 3.0
 * @remarks Stored string is replaced with @a uri. If @a uri is NULL, stored string is set
 * by NULL.
 *
 * @param[in] repr The handle to the Representation
 * @param[in] uri The URI of resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_repr_set_uri(iotcon_repr_h repr, const char *uri);
int iotcon_repr_get_uri(iotcon_repr_h repr, const char **uri);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Sets resource type list to the Representation.
 * @since_tizen 3.0
 * @remarks Stored list is replaced with @a types. If @a types is NULL, stored list is set
 * by NULL.
 * @param[in] repr The handle to the Representation
 * @param[in] types The resource type list
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_repr_set_resource_types(iotcon_repr_h repr, iotcon_resource_types_h types);
int iotcon_repr_get_resource_types(iotcon_repr_h repr, iotcon_resource_types_h *types);

int iotcon_repr_set_resource_interfaces(iotcon_repr_h repr, int ifaces);
int iotcon_repr_get_resource_interfaces(iotcon_repr_h repr);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Sets int value.
 * @since_tizen 3.0
 * @remarks Stored value is replaced with @a val.
 *
 * @param[in] repr The handle to the Representation
 * @param[in] type The resource type
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_repr_set_int(iotcon_repr_h repr, const char *key, int val);
int iotcon_repr_set_bool(iotcon_repr_h repr, const char *key, bool val);
int iotcon_repr_set_double(iotcon_repr_h repr, const char *key, double val);
int iotcon_repr_set_str(iotcon_repr_h repr, const char *key, char *val);
int iotcon_repr_set_list(iotcon_repr_h repr, const char *key, iotcon_list_h list);
int iotcon_repr_set_repr(iotcon_repr_h dest, const char *key, iotcon_repr_h src);
int iotcon_repr_set_null(iotcon_repr_h repr, const char *key);

int iotcon_repr_get_int(iotcon_repr_h repr, const char *key, int *val);
int iotcon_repr_get_bool(iotcon_repr_h repr, const char *key, bool *val);
int iotcon_repr_get_double(iotcon_repr_h repr, const char *key, double *val);
int iotcon_repr_get_str(iotcon_repr_h repr, const char *key, char **val);
int iotcon_repr_get_list(iotcon_repr_h repr, const char *key, iotcon_list_h *list);
int iotcon_repr_get_repr(iotcon_repr_h src, const char *key, iotcon_repr_h *dest);
bool iotcon_repr_is_null(iotcon_repr_h repr, const char *key);

int iotcon_repr_del_int(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_bool(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_double(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_str(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_list(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_repr(iotcon_repr_h repr, const char *key);
int iotcon_repr_del_null(iotcon_repr_h repr, const char *key);

int iotcon_repr_get_type(iotcon_repr_h repr, const char *key, int *type);

int iotcon_repr_append_child(iotcon_repr_h parent, iotcon_repr_h child);
typedef bool (*iotcon_children_fn)(iotcon_repr_h child, void *user_data);
int iotcon_repr_foreach_children(iotcon_repr_h parent, iotcon_children_fn fn,
		void *user_data);
unsigned int iotcon_repr_get_children_count(iotcon_repr_h parent);
int iotcon_repr_get_nth_child(iotcon_repr_h parent, int pos, iotcon_repr_h *child);

typedef int (*iotcon_repr_fn)(iotcon_repr_h repr, const char *key, void *user_data);
int iotcon_repr_foreach(iotcon_repr_h repr, iotcon_repr_fn fn, void *user_data);
unsigned int iotcon_repr_get_keys_count(iotcon_repr_h repr);

char* iotcon_repr_generate_json(iotcon_repr_h repr);

iotcon_list_h iotcon_list_new(iotcon_types_e type);
void iotcon_list_free(iotcon_list_h list);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Inserts integer value to list.
 * @since_tizen 3.0
 * @remarks If @a pos is negative, or is larger than the number of value in the list,
 * the new value is added on to the end of the list.
 *
 * @param[in] list The handle to the list
 * @param[in] val The integer
 * @param[in] pos The position to insert integer
 *
 * @return the (possibly changed) start of the list, otherwise a null pointer on failure
 */
int iotcon_list_insert_int(iotcon_list_h list, int val, int pos);
int iotcon_list_insert_bool(iotcon_list_h list, bool val, int pos);
int iotcon_list_insert_double(iotcon_list_h list, double val, int pos);
int iotcon_list_insert_str(iotcon_list_h list, char *val, int pos);
int iotcon_list_insert_list(iotcon_list_h list, iotcon_list_h val, int pos);
int iotcon_list_insert_repr(iotcon_list_h list, iotcon_repr_h val, int pos);

int iotcon_list_get_nth_int(iotcon_list_h list, int pos, int *val);
int iotcon_list_get_nth_bool(iotcon_list_h list, int pos, bool *val);
int iotcon_list_get_nth_double(iotcon_list_h list, int pos, double *val);
int iotcon_list_get_nth_str(iotcon_list_h list, int pos, const char **val);
int iotcon_list_get_nth_list(iotcon_list_h src, int pos, iotcon_list_h *dest);
int iotcon_list_get_nth_repr(iotcon_list_h list, int pos, iotcon_repr_h *repr);

int iotcon_list_del_nth_int(iotcon_list_h list, int pos);
int iotcon_list_del_nth_bool(iotcon_list_h list, int pos);
int iotcon_list_del_nth_double(iotcon_list_h list, int pos);
int iotcon_list_del_nth_str(iotcon_list_h list, int pos);
int iotcon_list_del_nth_list(iotcon_list_h list, int pos);
int iotcon_list_del_nth_repr(iotcon_list_h list, int pos);

int iotcon_list_get_type(iotcon_list_h list, int *type);
unsigned int iotcon_list_get_length(iotcon_list_h list);

typedef int (*iotcon_list_int_fn)(int pos, const int value, void *user_data);
int iotcon_list_foreach_int(iotcon_list_h list, iotcon_list_int_fn fn, void *user_data);
typedef int (*iotcon_list_bool_fn)(int pos, const bool value, void *user_data);
int iotcon_list_foreach_bool(iotcon_list_h list, iotcon_list_bool_fn fn, void *user_data);
typedef int (*iotcon_list_double_fn)(int pos, const double value, void *user_data);
int iotcon_list_foreach_double(iotcon_list_h list, iotcon_list_double_fn fn,
		void *user_data);
typedef int (*iotcon_list_str_fn)(int pos, const char *value, void *user_data);
int iotcon_list_foreach_str(iotcon_list_h list, iotcon_list_str_fn fn, void *user_data);
typedef int (*iotcon_list_list_fn)(int pos, iotcon_list_h value, void *user_data);
int iotcon_list_foreach_list(iotcon_list_h list, iotcon_list_list_fn fn, void *user_data);
typedef int (*iotcon_list_repr_fn)(int pos, iotcon_repr_h value, void *user_data);
int iotcon_list_foreach_repr(iotcon_list_h list, iotcon_list_repr_fn fn, void *user_data);

#endif /* __IOT_CONNECTIVITY_MANAGER_REPRESENTATION_H__ */
