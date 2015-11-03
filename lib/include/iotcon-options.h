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
#ifndef __IOT_CONNECTIVITY_MANAGER_STRUCT_OPTIONS_H__
#define __IOT_CONNECTIVITY_MANAGER_STRUCT_OPTIONS_H__

#include <iotcon-constant.h>

/**
 * @file iotcon-options.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_COMMON_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_COMMON_OPTIONS_MODULE Options
 *
 * @brief Iotcon Options provides API to manage options.
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_OPTIONS_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @{
 */

/**
 * @brief Creates a new option handle.
 *
 * @since_tizen 3.0
 *
 * @remarks You must destroy @a options by calling iotcon_options_destroy()
 * if @a options is no longer needed.
 *
 * @param[out] options A newly allocated option handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see iotcon_options_destroy()
 * @see iotcon_options_add()
 * @see iotcon_options_remove()
 * @see iotcon_options_lookup()
 */
int iotcon_options_create(iotcon_options_h *options);

/**
 * @brief Destroys an option handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] options The handle of the options
 *
 * @return void
 *
 * @see iotcon_options_create()
 * @see iotcon_options_add()
 * @see iotcon_options_remove()
 * @see iotcon_options_lookup()
 */
void iotcon_options_destroy(iotcon_options_h options);

/**
 * @brief Adds a new id and a correspoding data into the options.
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
 * @see iotcon_options_create()
 * @see iotcon_options_destroy()
 * @see iotcon_options_remove()
 * @see iotcon_options_lookup()
 */
int iotcon_options_add(iotcon_options_h options, unsigned short id,
		const char *data);

/**
 * @brief Removes the id and its associated data from the options.
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
 * @see iotcon_options_create()
 * @see iotcon_options_destroy()
 * @see iotcon_options_add()
 * @see iotcon_options_lookup()
 */
int iotcon_options_remove(iotcon_options_h options, unsigned short id);

/**
 * @brief Looks up data at the given id from the options.
 *
 * @since_tizen 3.0
 *
 * @remarks @a data must not be released using free().
 *
 * @param[in] options The handle of the options
 * @param[in] id The id of the option to lookup
 * @param[out] data Found data from options
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_options_create()
 * @see iotcon_options_destroy()
 * @see iotcon_options_add()
 * @see iotcon_options_remove()
 */
int iotcon_options_lookup(iotcon_options_h options, unsigned short id, char **data);

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
typedef int (*iotcon_options_foreach_cb)(unsigned short id, const char *data,
		void *user_data);

/**
 * @brief Gets all datas of the options by invoking the callback function.
 * @details iotcon_options_foreach_cb() will be called for each option.\n
 * If iotcon_options_foreach_cb() returns #IOTCON_FUNC_STOP, iteration will be stop.
 *
 * @since_tizen 3.0
 *
 * @param[in] options The handle of the options
 * @param[in] cb The callback function to get data
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_options_foreach_cb() will be called for each option.
 *
 * @see iotcon_options_foreach_cb()
 */
int iotcon_options_foreach(iotcon_options_h options, iotcon_options_foreach_cb cb,
		void *user_data);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_STRUCT_OPTIONS_H__ */
