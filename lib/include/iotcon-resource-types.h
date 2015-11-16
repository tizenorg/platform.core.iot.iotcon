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
#ifndef __IOT_CONNECTIVITY_MANAGER_STRUCT_RESOURCE_TYPES_H__
#define __IOT_CONNECTIVITY_MANAGER_STRUCT_RESOURCE_TYPES_H__

#include <iotcon-types.h>

/**
 * @file iotcon-resource-types.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_COMMON_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_COMMON_RESOURCE_TYPES_MODULE Resource Types
 *
 * @brief Iotcon Resource Types provides API to manage resource types.
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_RESOURCE_TYPES_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @{
 */

/**
 * @brief Creates a new resource types handle.
 *
 * @since_tizen 3.0
 *
 * @remarks You must destroy @a types by calling iotcon_resource_types_destroy()
 * if @a types is no longer needed.
 *
 * @param[out] types A newly allocated list of resource types handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_types_destroy()
 * @see iotcon_resource_types_add()
 * @see iotcon_resource_types_remove()
 * @see iotcon_resource_types_clone()
 */
int iotcon_resource_types_create(iotcon_resource_types_h *types);

/**
 * @brief Destroys a resource types handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] types The handle of the resource types
 *
 * @return void
 *
 * @see iotcon_resource_types_create()
 * @see iotcon_resource_types_add()
 * @see iotcon_resource_types_remove()
 * @see iotcon_resource_types_clone()
 */
void iotcon_resource_types_destroy(iotcon_resource_types_h types);

/**
 * @brief Inserts a resource type into the list.
 *
 * @since_tizen 3.0
 * @remarks The length of @a type should be less than or equal to 61.\n
 * The @a type must start with a lowercase alphabetic character, followed by a sequence
 * of lowercase alphabetic, numeric, ".", or "-" characters, and contains no white space.\n
 * Duplicate strings are not allowed.
 *
 * @param[in] types The handle of the resource types
 * @param[in] type The string data to insert into the resource types
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_types_create()
 * @see iotcon_resource_types_destroy()
 * @see iotcon_resource_types_remove()
 * @see iotcon_resource_types_clone()
 */
int iotcon_resource_types_add(iotcon_resource_types_h types, const char *type);

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
 * @see iotcon_resource_types_create()
 * @see iotcon_resource_types_destroy()
 * @see iotcon_resource_types_add()
 * @see iotcon_resource_types_clone()
 */
int iotcon_resource_types_remove(iotcon_resource_types_h types, const char *type);

/**
 * @brief Specifies the type of function passed to iotcon_resource_types_foreach()
 *
 * @since_tizen 3.0
 *
 * @param[in] type The value of the resource types
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_resource_types_foreach() will invoke this callback function.
 *
 * @see iotcon_resource_types_foreach()
 */
typedef bool (*iotcon_resource_types_foreach_cb)(const char *type, void *user_data);

/**
 * @brief Gets all of the resource types of the list by invoking the callback function.
 * @details iotcon_resource_types_foreach_cb() will be called for each type.\n
 * If iotcon_resource_types_foreach_cb() returns false, iteration will be stop.
 *
 * @since_tizen 3.0
 *
 * @param[in] types The handle of resource types
 * @param[in] cb The callback function to get data
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_resource_types_foreach() will be called for each type.
 *
 * @see iotcon_resource_types_foreach_cb()
 */
int iotcon_resource_types_foreach(iotcon_resource_types_h types,
		iotcon_resource_types_foreach_cb cb, void *user_data);

/**
 * @brief Clones the resource types handle.
 * @details Makes a deep copy of a source list of resource types.
 *
 * @since_tizen 3.0
 *
 * @remarks You must @a destroy dest by calling iotcon_resource_types_destroy()
 * if @a dest is no longer needed.
 *
 * @param[in] src The origin handle of the resource types
 * @param[out] dest Clone of a source list of resource types
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_resource_types_create()
 * @see iotcon_resource_types_destroy()
 * @see iotcon_resource_types_add()
 * @see iotcon_resource_types_remove()
 */
int iotcon_resource_types_clone(iotcon_resource_types_h src,
		iotcon_resource_types_h *dest);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_STRUCT_RESOURCE_TYPES_H__ */
