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
#ifndef __IOT_CONNECTIVITY_MANAGER_STRUCT_H__
#define __IOT_CONNECTIVITY_MANAGER_STRUCT_H__

#include <stdbool.h>

#include "iotcon-constant.h"

/**
 *
 * @addtogroup CAPI_IOT_CONNECTIVITY_MODULE
 *
 * @{
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE
 * @brief The handle of representation value.
 * @details iotcon_value_h is an opaque data structure to have variant datatype and
 * store values along with information about the type of that value.\n
 * The range of possible values is determined by the type.\n
 * The type of iotcon_value_h should be one of them\n
 * #IOTCON_TYPE_INT\n
 * #IOTCON_TYPE_BOOL\n
 * #IOTCON_TYPE_DOUBLE\n
 * #IOTCON_TYPE_STR\n
 * #IOTCON_TYPE_NULL\n
 * #IOTCON_TYPE_LIST\n
 * #IOTCON_TYPE_REPR
 *
 * @since_tizen 3.0
 */
typedef struct icl_value_s* iotcon_value_h;

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE
 * @brief The handle of list which is consist of iotcon_value_h type values.
 * @details iotcon_list_h is an opaque data structure to have iotcon_value_h type values.
 *
 * @since_tizen 3.0
 */
typedef struct icl_list_s* iotcon_list_h;

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE
 * @brief The handle of representation.
 * @details iotcon_repr_h is an opaque data structure to have uri_path, list of resource types,
 * interfaces and attribute value map.\n
 * It could contain other representation as children.\n
 * Attribute value map consists of a key and a value.\n
 * Datatype of the key is string and the value should be one of them\n
 * #IOTCON_TYPE_INT\n
 * #IOTCON_TYPE_BOOL\n
 * #IOTCON_TYPE_DOUBLE\n
 * #IOTCON_TYPE_STR\n
 * #IOTCON_TYPE_NULL\n
 * #IOTCON_TYPE_LIST\n
 * #IOTCON_TYPE_REPR
 *
 * @since_tizen 3.0
 */
typedef struct icl_repr_s* iotcon_repr_h;

/**
 * @brief The handle of notifications message.
 * @details iotcon_notimsg_h is an opaque data structure to notify message to observers.
 *
 * @since_tizen 3.0
 */
typedef struct icl_notify_msg* iotcon_notimsg_h;

/**
 * @brief The handle of presence handle
 * @details iotcon_presence_h is a handle of presence subscription.\n
 * It is used to cancel presence.
 *
 * @since_tizen 3.0
 */
typedef struct icl_presence* iotcon_presence_h;

/**
 * @brief The structure of device information
 *
 * @since_tizen 3.0
 */
typedef struct _device_info {
	char *device_name; /**< Name of the device */
} iotcon_device_info_s;

/**
 * @brief The structure of platform information
 *
 * @since_tizen 3.0
 */
typedef struct _platform_info {
	char *platform_id; /**< ID of the platform */
	char *manuf_name; /**< Manufacurer name of the platform */
	char *manuf_url; /**< Manufacurer url of the platform */
	char *model_number; /**< Model number of the platform */
	char *date_of_manufacture; /**< Date of manufacture of the platform */
	char *platform_ver; /**< Platform version of the platform */
	char *os_ver; /**< OS version of the platform */
	char *hardware_ver; /**< Hardware version of the platform */
	char *firmware_ver; /**< Firmware version of the platform */
	char *support_url; /**< Support url of the platform */
	char *system_time; /**< System time of the platform */
} iotcon_platform_info_s;

/**
 * @brief The handle of options
 * @details iotcon_options_h is an opaque data structure to have attribute value map
 * which consists of a key and a value.\n
 * Datatype of key is integer and value is string.\n
 *
 * @since_tizen 3.0
 */
typedef struct icl_options* iotcon_options_h;

/**
 * @brief Creates a new option handle.
 *
 * @since_tizen 3.0
 *
 * @return A newly allocated option handle, otherwise NULL on failure.
 * @retval iotcon_options_h Success
 * @retval NULL Failure
 *
 * @see iotcon_options_free()
 * @see iotcon_options_insert()
 * @see iotcon_options_delete()
 * @see iotcon_options_lookup()
 */
iotcon_options_h iotcon_options_new();

/**
 * @brief Free an option handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] options The handle of the options
 *
 * @return void
 *
 * @see iotcon_options_new()
 * @see iotcon_options_insert()
 * @see iotcon_options_delete()
 * @see iotcon_options_lookup()
 */
void iotcon_options_free(iotcon_options_h options);

/**
 * @brief Inserts a new id and a correspoding data into the options.
 *
 * @since_tizen 3.0
 * @remarks iotcon_options_h can have up to 2 options. \n
 * option id is always situated between 2048 and 3000. \n
 * Length of option data is less than or equal to 15.
 *
 * @param[in] options The handle of the options
 * @param[in] id The id of the option to insert
 * @param[in] data The string data to insert into the options
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_options_new()
 * @see iotcon_options_free()
 * @see iotcon_options_delete()
 * @see iotcon_options_lookup()
 */
int iotcon_options_insert(iotcon_options_h options, unsigned short id,
		const char *data);

/**
 * @brief Deletes the id and its associated data from the options.
 *
 * @since_tizen 3.0
 *
 * @param[in] options The handle of the options
 * @param[in] id The id of the option to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_options_new()
 * @see iotcon_options_free()
 * @see iotcon_options_insert()
 * @see iotcon_options_lookup()
 */
int iotcon_options_delete(iotcon_options_h options, unsigned short id);

/**
 * @brief Looks up data at the given id from the options.
 *
 * @since_tizen 3.0
 *
 * @param[in] options The handle of the options
 * @param[in] id The id of the option to lookup
 *
 * @return Found data from options on success, otherwise NULL on failure
 *
 * @see iotcon_options_new()
 * @see iotcon_options_free()
 * @see iotcon_options_insert()
 * @see iotcon_options_delete()
 */
const char* iotcon_options_lookup(iotcon_options_h options, unsigned short id);

/**
 * @brief Specifies the type of function passed to iotcon_options_foreach()
 *
 * @since_tizen 3.0
 *
 * @param[in] id The information of the option
 * @param[in] data The data of the option
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_options_foreach() will invoke this callback function.
 *
 * @see iotcon_options_foreach()
 */
typedef int (*iotcon_options_foreach_fn)(unsigned short id, const char *data,
		void *user_data);

/**
 * @brief Gets all datas of the options by invoking the callback function.
 * @details iotcon_options_foreach_fn() will be called for each option.\n
 * If iotcon_options_foreach_fn() returns #IOTCON_FUNC_STOP, iteration will be stop.
 *
 * @since_tizen 3.0
 *
 * @param[in] options The handle of the options
 * @param[in] fn The callback function to get data
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_options_foreach_fn() will be called for each option.
 *
 * @see iotcon_options_foreach_fn()
 */
int iotcon_options_foreach(iotcon_options_h options, iotcon_options_foreach_fn fn,
		void *user_data);

/**
 * @brief The handle of query
 * @details iotcon_query_h is an opaque data structure to have attribute value map
 * which consists of key and value.\n
 * Data ype of both key and value are string.\n
 * iotcon_query_h also have length.\n
 * The length is total length of all keys and values of map.\n
 * The length should be less than or equal to 64.
 *
 * @since_tizen 3.0
 */
typedef struct icl_query* iotcon_query_h;

/**
 * @brief Creates a new query handle.
 *
 * @since_tizen 3.0
 *
 * @return A newly allocated query handle, otherwise NULL on failure.
 * @retval iotcon_query_h Success
 * @retval NULL Failure
 *
 * @see iotcon_query_free()
 * @see iotcon_query_insert()
 * @see iotcon_query_delete()
 * @see iotcon_query_lookup()
 */
iotcon_query_h iotcon_query_new();

/**
 * @brief Free a query handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] query The handle of the query
 *
 * @return void
 *
 * @see iotcon_query_new()
 * @see iotcon_query_insert()
 * @see iotcon_query_delete()
 * @see iotcon_query_lookup()
 */
void iotcon_query_free(iotcon_query_h query);

/**
 * @brief Inserts a new key and correspoding value into the query.
 *
 * @since_tizen 3.0
 * @remarks The full length of query should be less than or equal to 64.
 *
 * @param[in] query The handle of the query
 * @param[in] key The key of the query to insert
 * @param[in] value The string data to insert into the query
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_query_new()
 * @see iotcon_query_free()
 * @see iotcon_query_delete()
 * @see iotcon_query_lookup()
 */
int iotcon_query_insert(iotcon_query_h query, const char *key, const char *value);

/**
 * @brief Deletes the key and its associated value from the query.
 *
 * @since_tizen 3.0
 *
 * @param[in] query The handle of the query
 * @param[in] key The key of the option to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_query_new()
 * @see iotcon_query_free()
 * @see iotcon_query_insert()
 * @see iotcon_query_lookup()
 */
int iotcon_query_delete(iotcon_query_h query, const char *key);

/**
 * @brief Lookup data at the given key from the query.
 *
 * @since_tizen 3.0
 *
 * @param[in] query The handle of the query
 * @param[in] key The key of the query to lookup
 *
 * @return Found data from query on success, otherwise a null pointer if fail to lookup
 *
 * @see iotcon_query_new()
 * @see iotcon_query_free()
 * @see iotcon_query_insert()
 * @see iotcon_query_delete()
 */
const char* iotcon_query_lookup(iotcon_query_h query, const char *key);

/**
 * @brief Specifies the type of function passed to iotcon_query_foreach()
 *
 * @since_tizen 3.0
 *
 * @param[in] key The key of the query
 * @param[in] value The value of the query
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_query_foreach() will invoke this callback function.
 *
 * @see iotcon_query_foreach()
 */
typedef int (*iotcon_query_foreach_fn)(const char *key, const char *value,
		void *user_data);

/**
 * @brief Gets all datas of the query by invoking the callback function.
 * @details iotcon_query_foreach_fn() will be called for each query.\n
 * If iotcon_query_foreach_fn() returns #IOTCON_FUNC_STOP, iteration will be stop.
 *
 * @since_tizen 3.0
 *
 * @param[in] query The handle of the query
 * @param[in] fn The callback function to get data
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_query_foreach_fn() will be called for each query.
 *
 * @see iotcon_query_foreach_fn()
 */
int iotcon_query_foreach(iotcon_query_h query, iotcon_query_foreach_fn fn,
		void *user_data);

/**
 * @brief The handle of resource types
 * @details iotcon_resource_types_h is an opaque data structure to have list of resource types.\n
 * A resource type is datatype of string.
 *
 * @since_tizen 3.0
 */
typedef struct icl_resource_types* iotcon_resource_types_h;

/**
 * @brief Creates a new resource types handle.
 *
 * @since_tizen 3.0
 *
 * @return A newly allocated list of resource types handle, otherwise NULL on failure.
 * @retval iotcon_resource_types_h Success
 * @retval NULL Failure
 *
 * @see iotcon_resource_types_free()
 * @see iotcon_resource_types_insert()
 * @see iotcon_resource_types_delete()
 * @see iotcon_resource_types_clone()
 */
iotcon_resource_types_h iotcon_resource_types_new();

/**
 * @brief Free a resource types handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] types The handle of the resource types
 *
 * @return void
 *
 * @see iotcon_resource_types_new()
 * @see iotcon_resource_types_insert()
 * @see iotcon_resource_types_delete()
 * @see iotcon_resource_types_clone()
 */
void iotcon_resource_types_free(iotcon_resource_types_h types);

/**
 * @brief Inserts a resource type into the list.
 *
 * @since_tizen 3.0
 * @remarks The length of resource type should be less than or equal to 61. \n
 * Duplicate strings are not allowed.
 *
 * @param[in] types The handle of the resource types
 * @param[in] type The string data to insert into the resource types
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_types_new()
 * @see iotcon_resource_types_free()
 * @see iotcon_resource_types_delete()
 * @see iotcon_resource_types_clone()
 */
int iotcon_resource_types_insert(iotcon_resource_types_h types, const char *type);

/**
 * @brief Delete a resource type form the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] types The handle of the resource types
 * @param[in] type The string data to delete from the resource types
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 *
 * @see iotcon_resource_types_new()
 * @see iotcon_resource_types_free()
 * @see iotcon_resource_types_insert()
 * @see iotcon_resource_types_clone()
 */
int iotcon_resource_types_delete(iotcon_resource_types_h types, const char *type);

/**
 * @brief Specifies the type of function passed to iotcon_resource_types_foreach()
 *
 * @since_tizen 3.0
 *
 * @param[in] type The value of the resource types
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_resource_types_foreach() will invoke this callback function.
 *
 * @see iotcon_resource_types_foreach()
 */
typedef int (*iotcon_resource_types_foreach_fn)(const char *type, void *user_data);

/**
 * @brief Gets all of the resource types of the list by invoking the callback function.
 * @details iotcon_resource_types_foreach_fn() will be called for each type.\n
 * If iotcon_resource_types_foreach_fn() returns #IOTCON_FUNC_STOP, iteration will be stop.
 *
 * @since_tizen 3.0
 *
 * @param[in] types The handle of resource types
 * @param[in] fn The callback function to get data
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_resource_types_foreach() will be called for each type.
 *
 * @see iotcon_resource_types_foreach_fn()
 */
int iotcon_resource_types_foreach(iotcon_resource_types_h types,
		iotcon_resource_types_foreach_fn fn, void *user_data);

/**
 * @brief Clones the resource types handle.
 * @details Makes a deep copy of a source list of resource types.
 *
 * @since_tizen 3.0
 *
 * @param[in] types The origin handle of the resource types
 *
 * @return Clone of a source list of resource types., otherwise NULL on failure
 * @retval iotcon_resource_types_h Success
 * @retval NULL Failure
 *
 * @see iotcon_resource_types_new()
 * @see iotcon_resource_types_free()
 * @see iotcon_resource_types_insert()
 * @see iotcon_resource_types_delete()
 */
iotcon_resource_types_h iotcon_resource_types_clone(iotcon_resource_types_h types);

/**
 * @brief The handle of observers.
 * @details The list of observer ids.
 *
 * @since_tizen 3.0
 */
typedef void* iotcon_observers_h;

/**
 * @brief Free a observers handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] observers The handle of the observers
 *
 * @return void
 *
 * @see iotcon_observers_append()
 * @see iotcon_observers_remove()
 */
void iotcon_observers_free(iotcon_observers_h observers);

/**
 * @brief Sets a observer id into the observers handle
 *
 * @since_tizen 3.0
 * @remarks  If you want to make a new list, then you should set NULL to @a observers.
 *
 * @param[in] observers The handle of the observers
 * @param[in] obs_id The id to be appended to observers
 *
 * @return New appended observers handle, otherwise a null pointer if error
 *
 * @see iotcon_observers_free()
 * @see iotcon_observers_remove()
 */
iotcon_observers_h iotcon_observers_append(iotcon_observers_h observers, int obs_id);

/**
 * @brief Remove id from the observers.
 *
 * @since_tizen 3.0
 *
 * @param[in] observers observers The handle of the observers
 * @param[in] obs_id The id to be removed from observers
 *
 * @return New deleted observers handle, otherwise a null pointer if error
 *
 * @see iotcon_observers_free()
 * @see iotcon_observers_append()
 */
iotcon_observers_h iotcon_observers_remove(iotcon_observers_h observers, int obs_id);

/**
 * @brief The handle of resource.
 * @details iotcon_resource_h is an opaque data structure to represent registered resource by server.
 * A resource has host, uri_path, resource types, interfaces and internal handle.\n
 * If observable attribute of resource is true, client can observe this resource.\n
 * When client request by CRUD functions, handler will be invoked if registered.
 * It could contain other resource as children.\n
 *
 * @since_tizen 3.0
 */
typedef struct icl_resource* iotcon_resource_h;

/**
 * @brief Gets the number of children resources of the resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] number The number of children resources
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_is_observable()
 */
int iotcon_resource_get_number_of_children(iotcon_resource_h resource, int *number);

/**
 * @brief Gets the child resource at the given index in the parent resource
 *
 * @since_tizen 3.0
 *
 * @param[in] parent The handle of the parent resource
 * @param[in] index The index of the child resource
 * @param[out] child The child resource at the index
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_is_observable()
 */
int iotcon_resource_get_nth_child(iotcon_resource_h parent, int index,
		iotcon_resource_h *child);

/**
 * @brief Gets an URI path of the resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] uri_path The URI path of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_is_observable()
 */
int iotcon_resource_get_uri_path(iotcon_resource_h resource, char **uri_path);

/**
 * @brief Get the list of types in the resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] types The types of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_interfaces()
 * @see iotcon_resource_is_observable()
 */
int iotcon_resource_get_types(iotcon_resource_h resource, iotcon_resource_types_h *types);

/**
 * @brief Get the interfaces of the resource
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] ifaces The interfaces of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_is_observable()
 */
int iotcon_resource_get_interfaces(iotcon_resource_h resource, int *ifaces);

/**
 * @brief Checks whether the resource is observable or not.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] observable The value of observable
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_get_number_of_children()
 * @see iotcon_resource_get_nth_child()
 * @see iotcon_resource_get_uri_path()
 * @see iotcon_resource_get_types()
 * @see iotcon_resource_get_interfaces()
 */
int iotcon_resource_is_observable(iotcon_resource_h resource, bool *observable);

/**
 * @brief The handle of client
 * @details When Client success to find out resource from remote server,
 * server's resource information is reorganized as iotcon_client_h by Iotcon.
 * Client can request CRUD to server by using this.\n
 * iotcon_client_h is an opaque data structure to have host, uri_path, resource types, interfaces,
 * options and server id.\n
 * If observable attribute is true, remote resource is observable.\n
 * When you observe remote resource, observe_handle will be set.
 *
 * @since_tizen 3.0
 */
typedef struct icl_remote_resource* iotcon_client_h;

/**
 * @brief Gets an URI path of the client
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the client
 * @param[out] uri_path The URI path of the client
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_get_host()
 * @see iotcon_client_get_server_id()
 * @see iotcon_client_get_types()
 * @see iotcon_client_get_interfaces()
 * @see iotcon_client_is_observable()
 * @see iotcon_client_set_options()
 */
int iotcon_client_get_uri_path(iotcon_client_h resource, char **uri_path);

/**
 * @brief Gets an host address of the client
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the client
 * @param[out] host The host address of the client
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_get_uri_path()
 * @see iotcon_client_get_server_id()
 * @see iotcon_client_get_types()
 * @see iotcon_client_get_interfaces()
 * @see iotcon_client_is_observable()
 * @see iotcon_client_set_options()
 */
int iotcon_client_get_host(iotcon_client_h resource, char **host);

/**
 * @brief Gets an server id of the client
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the client
 * @param[out] sid The server id of the client
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_get_uri_path()
 * @see iotcon_client_get_host()
 * @see iotcon_client_get_types()
 * @see iotcon_client_get_interfaces()
 * @see iotcon_client_is_observable()
 * @see iotcon_client_set_options()
 */
int iotcon_client_get_server_id(iotcon_client_h resource, char **sid);

/**
 * @brief Gets resource types of the client
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the client
 * @param[out] types The resource types of the client
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_get_uri_path()
 * @see iotcon_client_get_host()
 * @see iotcon_client_get_server_id()
 * @see iotcon_client_get_interfaces()
 * @see iotcon_client_is_observable()
 * @see iotcon_client_set_options()
 */
int iotcon_client_get_types(iotcon_client_h resource, iotcon_resource_types_h *types);

/**
 * @brief Gets resource interfaces of the client
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the client
 * @param[out] ifaces The resource interfaces of the client
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_get_uri_path()
 * @see iotcon_client_get_host()
 * @see iotcon_client_get_server_id()
 * @see iotcon_client_get_types()
 * @see iotcon_client_is_observable()
 * @see iotcon_client_set_options()
 */
int iotcon_client_get_interfaces(iotcon_client_h resource, int *ifaces);

/**
 * @brief Checks whether the client is observable or not.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[out] observable The value of observable
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_get_uri_path()
 * @see iotcon_client_get_host()
 * @see iotcon_client_get_server_id()
 * @see iotcon_client_get_types()
 * @see iotcon_client_get_interfaces()
 * @see iotcon_client_set_options()
 */
int iotcon_client_is_observable(iotcon_client_h resource, bool *observable);

/**
 * @brief Sets options into the client
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the client
 * @param[in] options The handle of the header options
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_get_uri_path()
 * @see iotcon_client_get_host()
 * @see iotcon_client_get_server_id()
 * @see iotcon_client_get_types()
 * @see iotcon_client_get_interfaces()
 * @see iotcon_client_is_observable()
 */
int iotcon_client_set_options(iotcon_client_h resource, iotcon_options_h options);

/**
* @brief The handle of request
* @details iotcon_request_h is an opaque data structure to request to a particular resource.\n
* iotcon_request_h is a data type of client's request which consists of header options,
* query, representation.
*
* @since_tizen 3.0
*/
typedef struct icl_resource_request* iotcon_request_h;

/**
 * @brief Gets an URI path of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] uri_path The URI path of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_types()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observer_action()
 * @see iotcon_request_get_observer_id()
 */
int iotcon_request_get_uri_path(iotcon_request_h request, char **uri_path);

/**
 * @brief Gets an representation of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] repr The representation of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_uri_path()
 * @see iotcon_request_get_types()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observer_action()
 * @see iotcon_request_get_observer_id()
 */
int iotcon_request_get_representation(iotcon_request_h request, iotcon_repr_h *repr);

/**
 * @brief Get types of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] types The types of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_uri_path()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observer_action()
 * @see iotcon_request_get_observer_id()
 */
int iotcon_request_get_types(iotcon_request_h request, int *types);

/**
 * @brief Get options of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] options The options of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_uri_path()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_types()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observer_action()
 * @see iotcon_request_get_observer_id()
 */
int iotcon_request_get_options(iotcon_request_h request, iotcon_options_h *options);

/**
 * @brief Get query of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] query The query of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_uri_path()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_types()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_observer_action()
 * @see iotcon_request_get_observer_id()
 */
int iotcon_request_get_query(iotcon_request_h request, iotcon_query_h *query);

/**
 * @brief Get observation action of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] action The observation action of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_uri_path()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_types()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observer_id()
 */
int iotcon_request_get_observer_action(iotcon_request_h request,
		iotcon_observe_action_e *action);

/**
 * @brief Get observation id of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] observer_id The id of the observer
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_uri_path()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_types()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observer_action()
 */
int iotcon_request_get_observer_id(iotcon_request_h request, int *observer_id);

/**
* @brief The handle of response
* @details iotcon_response_h is an opaque data structure to respond to client.\n
* iotcon_response_h is a data type of server's response which consists of result,
* header options, query, representation.
*
*
* @since_tizen 3.0
*/
typedef struct icl_resource_response* iotcon_response_h;

/**
 * @brief Creates a response handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] request_h The handle of received request handle
 *
 * @return Generated response handle, otherwise a null pointer if a allocation error
 * @retval iotcon_response_h Success
 * @retval NULL Failure
 *
 * @see iotcon_response_free()
 * @see iotcon_response_set()
 */
iotcon_response_h iotcon_response_new(iotcon_request_h request_h);

/**
 * @brief Free a response handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 *
 * @see iotcon_response_new()
 * @see iotcon_response_set()
 */
void iotcon_response_free(iotcon_response_h resp);

/**
 * @brief Sets values into the response
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 * @param[in] prop The property of the response to set
 * @param[in] ... arguments, as per format_string
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_response_new()
 * @see iotcon_response_free()
 */
int iotcon_response_set(iotcon_response_h resp, iotcon_response_property_e prop, ...);


/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_STRUCT_H__ */
