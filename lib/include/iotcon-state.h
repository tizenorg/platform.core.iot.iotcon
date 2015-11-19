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
#ifndef __IOT_CONNECTIVITY_MANAGER_STRUCT_STATE_H__
#define __IOT_CONNECTIVITY_MANAGER_STRUCT_STATE_H__

#include <iotcon-types.h>

/**
 * @file iotcon-state.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_MODULE State
 *
 * @brief Iotcon State provides API to manage state.
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_MODULE_OVERVIEW Overview
 * The iotcon state API provides string key based hash table.
 *
 * Example :
 * @code
#include <iotcon.h>
...
static void _request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
{
	int ret;
	int types;

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret)
		return;

	if (IOTCON_REQUEST_GET & types) {
		iotcon_response_h response = NULL;
		iotcon_representation_h representation = NULL;
		iotcon_state_h state = NULL;

		ret = iotcon_response_create(request, &response);
		if (IOTCON_ERROR_NONE != ret)
			return;

		ret = iotcon_representation_create(&representation);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_response_destroy(resopnse);
			return;
		}

		...

		ret = iotcon_state_create(&state);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_state_set_bool(state, "power", true);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = itocon_state_set_int(state, "brightness", 75);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_representation_set_state(representation, state);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		...

		ret = iotcon_response_set_representation(response, IOTCON_INTERFACE_DEFAULT,
				representation);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_response_send(response);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		iotcon_state_destroy(state);
		iotcon_representation_destroy(representation);
		iotcon_response_destroy(resopnse);
	}
	...
}
 * @endcode
 *
 * @{
 */

/**
 * @brief Creates a new state handle.
 *
 * @since_tizen 3.0
 *
 * @remarks You must destroy @a state by calling iotcon_state_destroy()
 * if @a state is no longer needed.
 *
 * @param[out] state A newly allocated state handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_state_destroy()
 */
int iotcon_state_create(iotcon_state_h *state);

/**
 * @brief Destroys a state.
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
 * @brief Clones a state handle.
 *
 * @since_tizen 3.0
 *
 * @remarks You must destroy @a state_clone by calling iotcon_state_destroy()
 * if @a state_clone is no longer needed.
 *
 * @param[in] state The state handle
 * @param[out] state_clone The cloned state handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_state_create()
 * @see iotcon_state_destroy()
 */
int iotcon_state_clone(iotcon_state_h state, iotcon_state_h *state_clone);

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
 * @remarks @a val must not be released using free().
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
 * @remarks @a list must not be released using iotcon_list_destroy().
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
 * @remarks @a state must not be released using iotcon_state_destroy().
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
 * @brief Unsets the key and its associated value from the state.
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
 */
int iotcon_state_unset(iotcon_state_h state, const char *key);

/**
 * @brief Gets the type of a value at the given key.
 * @details It gets the data type of value related the @a key in @a state.
 * The data type could be one of #iotcon_type_e.
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
int iotcon_state_get_type(iotcon_state_h state, const char *key,
		iotcon_type_e *type);

/**
 * @brief Specifies the type of function passed to iotcon_state_foreach()
 *
 * @since_tizen 3.0
 *
 * @param[in] state The state handle
 * @param[in] key The key
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_state_foreach() will invoke this callback function.
 *
 * @see iotcon_state_foreach()
 */
typedef bool (*iotcon_state_cb)(iotcon_state_h state, const char *key, void *user_data);

/**
 * @brief Calls a function for each element of state.
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
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_STRUCT_STATE_H__ */
