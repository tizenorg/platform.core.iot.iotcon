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

/**
 *
 * @addtogroup CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE
 *
 * @{
 */

/**
 * @brief Creates a new representation handle.
 *
 * @since_tizen 3.0
 *
 * @param[out] repr A newly allocated representation handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_representation_destroy()
 */
int iotcon_representation_create(iotcon_representation_h *ret_repr);

/**
 * @brief Frees a representation.
 * @details Releases a @a representation and its internal data.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle to free
 *
 * @return void
 *
 * @see iotcon_representation_create()
 */
void iotcon_representation_destroy(iotcon_representation_h repr);

/**
 * @brief Clones from the source representation.
 * @details Makes a deep copy of a source representation.
 *
 * @since_tizen 3.0
 *
 * @param[in] src Source of representation to be copied
 * @param[out] dest Clone of a source representation
 *
 * @return Clone of a source representation, otherwise NULL on failure
 * @retval iotcon_representation_h Success
 * @retval NULL Failure
 */
int iotcon_representation_clone(const iotcon_representation_h src, iotcon_representation_h *dest);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Appends resource type name.
 *
 * @since_tizen 3.0
 * @remarks Stored string is replaced with @a uri_path. If @a uri_path is NULL, stored string
 * is set by NULL.
 *
 * @param[in] repr The handle to the Representation
 * @param[in] uri_path The URI of resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_set_uri_path(iotcon_representation_h repr, const char *uri_path);

/**
 * @brief Gets an URI path from the representation.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[out] uri_path The URI path to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_get_uri_path(iotcon_representation_h repr, const char **uri_path);

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Sets resource type list to the Representation.
 *
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
int iotcon_representation_set_resource_types(iotcon_representation_h repr, iotcon_resource_types_h types);

/**
 * @brief Gets list of resource type from the representation.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[out] types The list of resource types to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_get_resource_types(iotcon_representation_h repr, iotcon_resource_types_h *types);

/**
 * @brief Sets interfaces to the representation.
 * @details If you set new interfaces, current interfaces value will be replaced with @a ifaces.\n
 * @a ifaces can be consist of multiple interface like
 * IOTCON_INTERFACE_LINK | IOTCON_INTERFACE_BATCH.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[in] ifaces The interfaces to set
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_set_resource_interfaces(iotcon_representation_h repr, int ifaces);

/**
 * @brief Gets resource interfaces from the representation.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[out] ifaces The interfaces to get
 *
 * @return Interfaces to get. Interfaces may contain multiple interfaces.
 * @retval #IOTCON_INTERFACE_NONE  Not set
 * @retval Bitwise OR value which consists of iotcon_interface_e items
 */
int iotcon_representation_get_resource_interfaces(iotcon_representation_h repr, int *ifaces);

/**
 * @brief Sets a new state handle into the representation.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[in] state The state handle to set newly
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_set_state(iotcon_representation_h repr, iotcon_state_h state);

/**
 * @brief Gets a state handle in the representation.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[in] state The state handle to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_get_state(iotcon_representation_h repr, iotcon_state_h *state);

/**
 * @brief Deletes state handle in the representation.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_del_state(iotcon_representation_h repr);

/**
 * @brief Creates a new state handle.
 *
 * @since_tizen 3.0
 *
 * @param[out] state A newly allocated state handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_state_destroy()
 */
int iotcon_state_create(iotcon_state_h *ret_state);

/**
 * @brief Frees a state.
 * @details Releases a @a state and its internal data.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle to free
 *
 * @return void
 *
 * @see iotcon_state_create()
 */
void iotcon_state_destroy(iotcon_state_h state);

/**
 * @brief Sets a new key and integer value into the representation.
 * @details If @a key is already exists, current value will be replaced with new @a val.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[in] val The value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_set_int(iotcon_state_h state, const char *key, int val);

/**
 * @brief Sets a new key and boolean value into the representation.
 * @details If @a key is already exists, current value will be replaced with new @a val.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[in] val The value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_set_bool(iotcon_state_h state, const char *key, bool val);

/**
 * @brief Sets a new key and double value into the representation.
 * @details If @a key is already exists, current value will be replaced with new @a val.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[in] val The value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_set_double(iotcon_state_h state, const char *key, double val);

/**
 * @brief Sets a new key and string value into the representation.
 * @details If @a key is already exists, current value will be replaced with new @a val.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[in] val The value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_set_str(iotcon_state_h state, const char *key, char *val);

/**
 * @brief Sets a new key and list value into the representation.
 * @details If @a key is already exists, current list will be replaced with new @a list.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[in] list The value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_set_list(iotcon_state_h state, const char *key, iotcon_list_h list);

/**
 * @brief Sets a new key and state value into the representation.
 * @details If @a key is already exists, current state will be replaced with new @a src.
 *
 * @since_tizen 3.0
 *
 * @param[in] dest The state handle
 * @param[in] key The key
 * @param[in] src The state handle to set newly
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_set_state(iotcon_state_h dest, const char *key, iotcon_state_h src);

/**
 * @brief Sets a new key with NULL value into the representation.
 * @details If @a key is already exists, current value will be replaced with NULL
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key to be set NULL
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_set_null(iotcon_state_h state, const char *key);

/**
 * @brief Gets the integer value from the given key.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[out] val The integer value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_get_int(iotcon_state_h state, const char *key, int *val);

/**
 * @brief Gets the boolean value from the given key.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[out] val The boolean value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_get_bool(iotcon_state_h state, const char *key, bool *val);

/**
 * @brief Gets the double value from the given key.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[out] val The double value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_get_double(iotcon_state_h state, const char *key, double *val);

/**
 * @brief Gets the string value from the given key.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[out] val The string value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_get_str(iotcon_state_h state, const char *key, char **val);

/**
 * @brief Gets the list value from the given key.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[out] list The list value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_get_list(iotcon_state_h state, const char *key, iotcon_list_h *list);

/**
 * @brief Gets the state value from the given key.
 *
 * @since_tizen 3.0
 *
 * @param[in] src The state handle
 * @param[in] key The key
 * @param[out] dest The state value at the key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_get_state(iotcon_state_h src, const char *key, iotcon_state_h *dest);

/**
 * @brief Checks whether the value of given key is NULL or not.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[out] is_null true if the type of the given key is null, otherwise false
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_is_null(iotcon_state_h state, const char *key, bool *is_null);

/**
 * @brief Deletes the key and its associated integer value from the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_del_int(iotcon_state_h state, const char *key);

/**
 * @brief Deletes the key and its associated boolean value from the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_del_bool(iotcon_state_h state, const char *key);

/**
 * @brief Deletes the key and its associated double value from the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_del_double(iotcon_state_h state, const char *key);

/**
 * @brief Deletes the key and its associated string value from the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_del_str(iotcon_state_h state, const char *key);

/**
 * @brief Deletes the key and its associated list value from the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_del_list(iotcon_state_h state, const char *key);

/**
 * @brief Deletes the key and its associated state value from the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_del_state(iotcon_state_h state, const char *key);

/**
 * @brief Deletes the key from the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_state_del_null(iotcon_state_h state, const char *key);

/**
 * @brief Gets the type of a value at the given key.
 * @details It gets the data type of value related the @a key in @a state.
 * The data type could be one of #iotcon_types_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[out] type The data type of value related the key in state handle.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 */
int iotcon_state_get_type(iotcon_state_h state, const char *key, int *type);

/**
 * @brief Appends a new child representation on to the end of the parent representation
 * @details Duplicated child representation is allowed to append.
 *
 * @since_tizen 3.0
 *
 * @param[in] parent The parent representation handle
 * @param[in] child The child representation handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_append_child(iotcon_representation_h parent,
		iotcon_representation_h child);

/**
 * @brief Removes a child representation from parent representation without freeing.
 *
 * @since_tizen 3.0
 *
 * @param[in] parent The parent representation handle
 * @param[in] child The child representation handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_remove_child(iotcon_representation_h parent,
		iotcon_representation_h child);

/**
 * @brief Specifies the type of function passed to iotcon_representation_foreach_children()
 *
 * @since_tizen 3.0
 *
 * @param[in] child The child representation handle
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_FUNC_CONTINUE  Continue to iterate next child representation
 * @retval #IOTCON_FUNC_STOP  Stop to iterate children representation
 *
 * @pre iotcon_representation_foreach_children() will invoke this callback function.
 *
 * @see iotcon_representation_foreach_children()
 *
 */
typedef bool (*iotcon_children_cb)(iotcon_representation_h child, void *user_data);

/**
 * @brief Call a function for each children representation of parent.
 * @details iotcon_children_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] parent The parent representation handle
 * @param[in] cb The callback function to invoke
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_children_cb() will be called for each child.
 *
 * @see iotcon_children_cb()
 */
int iotcon_representation_foreach_children(iotcon_representation_h parent, iotcon_children_cb cb,
		void *user_data);

/**
 * @brief Gets the number of children representation in the parent representation
 *
 * @since_tizen 3.0
 *
 * @param[in] parent The parent representation handle
 * @param[out] count The number of children representation
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_get_children_count(iotcon_representation_h parent, unsigned int *count);

/**
 * @brief Gets the child representation at the given position.
 * @details Iterates over the parent until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] parent The parent representation handle
 * @param[in] pos The position of the child representation
 * @param[out] child The handle to the child representation
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 */
int iotcon_representation_get_nth_child(iotcon_representation_h parent, int pos, iotcon_representation_h *child);

/**
 * @brief Specifies the type of function passed to iotcon_state_foreach()
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_CONTINUE  Continue to iterate next key
 * @retval #IOTCON_FUNC_STOP  Stop to iterate key
 *
 * @pre iotcon_state_foreach() will invoke this callback function.
 *
 * @see iotcon_state_foreach()
 */
typedef int (*iotcon_state_cb)(iotcon_state_h state, const char *key, void *user_data);

/**
 * @brief Call a function for each element of state.
 * @details iotcon_state_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] cb The callback function to invoke
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_state_cb() will be called for each child.
 *
 * @see iotcon_state_cb()
 */
int iotcon_state_foreach(iotcon_state_h state, iotcon_state_cb cb, void *user_data);

/**
 * @brief  Gets the number of keys in the state.
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[out] count The number of keys
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_state_get_keys_count(iotcon_state_h state, unsigned int *count);

/**
 * @brief Creates a new list handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] type The type of list
 * @param[out] list A newly allocated list handle
 *
 * @return A newly allocated list handle, otherwise NULL on failure.
 * @retval iotcon_list_h Success
 * @retval NULL Failure
 */
int iotcon_list_create(iotcon_types_e type, iotcon_list_h *list);

/**
 * @brief Frees a list handle.
 * @details Releases a @a list and its internal data.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 *
 * @return void
 */
void iotcon_list_destroy(iotcon_list_h list);

/**
 * @brief Inserts a new element integer value into the list at the given position.
 * @details If @a pos is negative, or is larger than the number of elements in the list,
 * the new value is added on to the end of the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] val The new integer value
 * @param[in] pos The position to insert value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_insert_int(iotcon_list_h list, int val, int pos);

/**
 * @brief Inserts a new element boolean value into the list at the given position.
 * @details If @a pos is negative, or is larger than the number of elements in the list,
 * the new value is added on to the end of the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] val The new boolean value
 * @param[in] pos The position to insert value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_insert_bool(iotcon_list_h list, bool val, int pos);

/**
 * @brief Inserts a new element double value into the list at the given position.
 * @details If @a pos is negative, or is larger than the number of elements in the list,
 * the new value is added on to the end of the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] val The new double value
 * @param[in] pos The position to insert value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_insert_double(iotcon_list_h list, double val, int pos);

/**
 * @brief Inserts a new element string value into the list at the given position.
 * @details If @a pos is negative, or is larger than the number of elements in the list,
 * the new value is added on to the end of the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] val The new char value
 * @param[in] pos The position to insert value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_insert_str(iotcon_list_h list, char *val, int pos);

/**
 * @brief Inserts a new element list into the list at the given position.
 * @details If @a pos is negative, or is larger than the number of elements in the list,
 * the new value is added on to the end of the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] val The new list value
 * @param[in] pos The position to insert value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_insert_list(iotcon_list_h list, iotcon_list_h val, int pos);

/**
 * @brief Inserts a new element state value into the list at the given position.
 * @details If @a pos is negative, or is larger than the number of elements in the list,
 * the new value is added on to the end of the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] val The new state value
 * @param[in] pos The position to insert value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_insert_state(iotcon_list_h list, iotcon_state_h val, int pos);

/**
 * @brief Gets the integer value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] val The integer value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_int(iotcon_list_h list, int pos, int *val);

/**
 * @brief Gets the boolean value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] val The boolean value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_bool(iotcon_list_h list, int pos, bool *val);

/**
 * @brief Gets the double value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] val The double value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_double(iotcon_list_h list, int pos, double *val);

/**
 * @brief Gets the string value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] val The string value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_str(iotcon_list_h list, int pos, const char **val);

/**
 * @brief Gets the list value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] src The list handle
 * @param[in] pos The position
 * @param[out] dest The list value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_list(iotcon_list_h src, int pos, iotcon_list_h *dest);

/**
 * @brief Gets the state value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] state The state value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_state(iotcon_list_h list, int pos, iotcon_state_h *state);

/**
 * @brief Deletes the integer value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_del_nth_int(iotcon_list_h list, int pos);

/**
 * @brief Deletes the boolean value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_del_nth_bool(iotcon_list_h list, int pos);

/**
 * @brief Deletes the double value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_del_nth_double(iotcon_list_h list, int pos);

/**
 * @brief Deletes the string value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_del_nth_str(iotcon_list_h list, int pos);

/**
 * @brief Deletes the list value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_del_nth_list(iotcon_list_h list, int pos);

/**
 * @brief Deletes the state value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_del_nth_state(iotcon_list_h list, int pos);

/**
 * @brief Gets the type of the list.
 * @details It gets the data type of value related the @a key in @a state.
 * The data type could be one of #iotcon_types_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[out] type The data type of list.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_list_get_type(iotcon_list_h list, int *type);

/**
 * @brief Gets the number of elements in a list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 * @param[out] length The length of list
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_list_get_length(iotcon_list_h list, unsigned int *length);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_int()
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the integer value (0 being the first)
 * @param[in] value The integer value
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_list_foreach_int() will invoke this callback function.
 *
 * @see iotcon_list_foreach_int()
 */
typedef int (*iotcon_list_int_cb)(int pos, const int value, void *user_data);

/**
 * @brief Gets all integer values of the given list by invoking the callback function.
 * @details iotcon_list_int_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 * @param[in] cb The callback function to get each integer value
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_int_cb() will be called for each item.
 *
 * @see iotcon_list_int_cb()
 */
int iotcon_list_foreach_int(iotcon_list_h list, iotcon_list_int_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_bool()
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the boolean value (0 being the first)
 * @param[in] value The boolean value
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_list_foreach_bool() will invoke this callback function.
 *
 * @see iotcon_list_foreach_bool()
 */
typedef int (*iotcon_list_bool_cb)(int pos, const bool value, void *user_data);

/**
 * @brief Gets all boolean values of the given list by invoking the callback function.
 * @details iotcon_list_bool_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 * @param[in] cb The callback function to get each boolean value
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_bool_cb() will be called for each item.
 *
 * @see iotcon_list_bool_cb()
 */
int iotcon_list_foreach_bool(iotcon_list_h list, iotcon_list_bool_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_double()
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the double value (0 being the first)
 * @param[in] value The double value
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_list_foreach_double() will invoke this callback function.
 *
 * @see iotcon_list_foreach_double()
 */
typedef int (*iotcon_list_double_cb)(int pos, const double value, void *user_data);

/**
 * @brief Gets all double values of the given list by invoking the callback function.
 * @details iotcon_list_double_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 * @param[in] cb The callback function to get each double value
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_double_cb() will be called for each item.
 *
 * @see iotcon_list_double_cb()
 */
int iotcon_list_foreach_double(iotcon_list_h list, iotcon_list_double_cb cb,
		void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_str()
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the string value (0 being the first)
 * @param[in] value The string value
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_list_foreach_str() will invoke this callback function.
 *
 * @see iotcon_list_foreach_str()
 */
typedef int (*iotcon_list_str_cb)(int pos, const char *value, void *user_data);

/**
 * @brief Gets all string values of the given list by invoking the callback function.
 * @details iotcon_list_str_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 * @param[in] cb The callback function to get each string value
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_str_cb() will be called for each item.
 *
 * @see iotcon_list_str_cb()
 */
int iotcon_list_foreach_str(iotcon_list_h list, iotcon_list_str_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_list()
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the list value (0 being the first)
 * @param[in] value The list value
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_list_foreach_list() will invoke this callback function.
 *
 * @see iotcon_list_foreach_list()
 */
typedef int (*iotcon_list_list_cb)(int pos, iotcon_list_h value, void *user_data);

/**
 * @brief Gets all sub lists of the given list by invoking the callback function.
 * @details iotcon_list_list_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the origin list
 * @param[in] cb The callback function to get each sub list
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_list_cb() will be called for each item.
 *
 * @see iotcon_list_list_cb()
 */
int iotcon_list_foreach_list(iotcon_list_h list, iotcon_list_list_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_state()
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the state value (0 being the first)
 * @param[in] value The state value
 * @param[in] user_data The user data to pass to the function
 *
 * @return #IOTCON_FUNC_CONTINUE to continue with the next function of the loop,
 * otherwise #IOTCON_FUNC_STOP to break out of the loop
 * @retval #IOTCON_FUNC_STOP  stop to call next function
 * @retval #IOTCON_FUNC_CONTINUE  continue to call next function
 *
 * @pre iotcon_list_foreach_state() will invoke this callback function.
 *
 * @see iotcon_list_foreach_state()
 */
typedef int (*iotcon_list_state_cb)(int pos, iotcon_state_h value, void *user_data);

/**
 * @brief Gets all state of the given list by invoking the callback function.
 * @details iotcon_list_state_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 * @param[in] cb The callback function to get each state
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_state_cb() will be called for each item.
 *
 * @see iotcon_list_state_cb()
 */
int iotcon_list_foreach_state(iotcon_list_h list, iotcon_list_state_cb cb, void *user_data);

/**
 * @}
 */
#endif /* __IOT_CONNECTIVITY_MANAGER_REPRESENTATION_H__ */
