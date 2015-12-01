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
#ifndef __IOT_CONNECTIVITY_MANAGER_STRUCT_REPRESENTATION_H__
#define __IOT_CONNECTIVITY_MANAGER_STRUCT_REPRESENTATION_H__

#include <iotcon-types.h>

/**
 * @file iotcon-representation.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_COMMON_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_MODULE Representation
 *
 * @brief Iotcon Representation provides API to manage representation
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_MODULE_OVERVIEW Overview
 * The Iotcon Representation API provides data type of resp_repr handling.\n
 * A resp_repr is a payload of a request or a response.\n
 * It has uri_path, interface, list of resource types and its attributes.\n
 * Attributes have capabilties to store and retrieve integer, boolean, double, string, list, null,
 * resp_repr.\n
 * A list is a container that includes number of datas of same type.\n
 * It has capabilties to store and retrieve integer, boolean, double, string, list, null, resp_repr.
 *
 * Example :
 *@code
#include <iotcon.h>
...
{
	int ret;
	iotcon_representation_h repr;
	iotcon_resource_types_h types;
	iotcon_list_h bright_step_list;

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_representation_set_uri_path(resp_repr, "/a/light");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_resource_types_create(&types);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_resource_types_add(types, "core.light");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_representation_set_resource_types(resp_repr, types);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_representation_set_resource_interfaces(resp_repr, IOTCON_INTERFACE_LINK);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_add_str(resp_repr, "type", "lamp");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_add_str(resp_repr, "where", "desk");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_add_double(resp_repr, "default_bright", 200.0);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_add_str(resp_repr, "unit", "lux");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_add_bool(resp_repr, "bright_step", true);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_list_create(IOTCON_TYPE_DOUBLE, &bright_step_list);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_list_add_double(bright_step_list, 100.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_destroy(bright_step_list);
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_list_add_double(bright_step_list, 200.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_destroy(bright_step_list);
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_list_add_double(bright_step_list, 300.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_destroy(bright_step_list);
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_list_add_double(bright_step_list, 400.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_destroy(bright_step_list);
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_list_add_double(bright_step_list, 500.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_destroy(bright_step_list);
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_add_list(resp_repr, "bright_step_list", bright_step_list);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_destroy(bright_step_list);
		iotcon_resource_types_destroy(types);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	iotcon_list_destroy(bright_step_list);
	iotcon_resource_types_destroy(types);
	iotcon_representation_destroy(resp_repr);
}
 * @endcode
 * @{
 */

/**
 * @brief Creates a new representation handle.
 *
 * @since_tizen 3.0
 *
 * @remarks You must destroy @a repr by calling iotcon_representation_destroy()
 * if @a repr is no longer needed.
 *
 * @param[out] repr A newly allocated representation handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_representation_destroy()
 */
int iotcon_representation_create(iotcon_representation_h *repr);

/**
 * @brief Destroys a representation.
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
 * @remarks You must destroy @a dest by calling iotcon_representation_destroy()
 * if @a dest is no longer needed.
 *
 * @param[in] src Source of representation to be copied
 * @param[out] dest Clone of a source representation
 *
 * @return Clone of a source representation, otherwise NULL on failure
 * @retval iotcon_representation_h Success
 * @retval NULL Failure
 */
int iotcon_representation_clone(const iotcon_representation_h src,
		iotcon_representation_h *dest);

/**
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
int iotcon_representation_set_uri_path(iotcon_representation_h repr,
		const char *uri_path);

/**
 * @brief Gets an URI path from the representation.
 *
 * @since_tizen 3.0
 *
 * @remarks @a uri_path must not be released using free().
 *
 * @param[in] repr The representation handle
 * @param[out] uri_path The URI path to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_get_uri_path(iotcon_representation_h repr, char **uri_path);

/**
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
int iotcon_representation_set_resource_types(iotcon_representation_h repr,
		iotcon_resource_types_h types);

/**
 * @brief Gets list of resource type from the representation.
 *
 * @since_tizen 3.0
 *
 * @remarks @a types must not be released using iotcon_resource_types_destroy().
 *
 * @param[in] repr The representation handle
 * @param[out] types The list of resource types to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_get_resource_types(iotcon_representation_h repr,
		iotcon_resource_types_h *types);

/**
 * @brief Sets interfaces to the representation.
 * @details If you set new interfaces, current interfaces value will be replaced with @a ifaces.\n
 * @a ifaces can be consist of multiple interface like
 * IOTCON_INTERFACE_LINK | IOTCON_INTERFACE_BATCH.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[in] ifaces The interfaces to set\n Set of #iotcon_interface_e
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_representation_set_resource_interfaces(iotcon_representation_h repr,
		int ifaces);

/**
 * @brief Gets resource interfaces from the representation.
 * @details @a ifaces can contain multiple interfaces like
 * IOTCON_INTERFACE_LINK | IOTCON_INTERFACE_BATCH.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The representation handle
 * @param[out] ifaces The interfaces to get\n Set of #iotcon_interface_e
 *
 * @return Interfaces to get. Interfaces may contain multiple interfaces.
 * @retval #IOTCON_INTERFACE_NONE  Not set
 * @retval Bitwise OR value which consists of iotcon_interface_e items
 */
int iotcon_representation_get_resource_interfaces(iotcon_representation_h repr,
		int *ifaces);

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
 * @brief Adds a new child representation on to the end of the parent representation
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
int iotcon_representation_add_child(iotcon_representation_h parent,
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
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_representation_foreach_children() will invoke this callback function.
 *
 * @see iotcon_representation_foreach_children()
 *
 */
typedef bool (*iotcon_children_cb)(iotcon_representation_h child, void *user_data);

/**
 * @brief Calls a function for each children representation of parent.
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
int iotcon_representation_foreach_children(iotcon_representation_h parent,
		iotcon_children_cb cb, void *user_data);

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
int iotcon_representation_get_children_count(iotcon_representation_h parent,
		unsigned int *count);

/**
 * @brief Gets the child representation at the given position.
 * @details Iterates over the parent until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @remarks @a child must not be released using iotcon_representation_destroy().
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
int iotcon_representation_get_nth_child(iotcon_representation_h parent, int pos,
		iotcon_representation_h *child);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_STRUCT_REPRESENTATION_H__ */
