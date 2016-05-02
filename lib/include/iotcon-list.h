/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#ifndef __IOT_CONNECTIVITY_STRUCT_LIST_H__
#define __IOT_CONNECTIVITY_STRUCT_LIST_H__

#include <iotcon-types.h>

/**
 * @file iotcon-list.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_LIST_MODULE List
 *
 * @brief Iotcon List provides API to get data from list and set data to list.
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_LIST_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_LIST_MODULE_OVERVIEW Overview
 * The iotcon list API provides list of bool, integer, double, string, byte string, list and state handle.
 *
 * Example :
 * @code
static void _request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
{
	int ret;
	iot types;

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret)
		return;

	if (IOTCON_REQUEST_GET & types) {
		iotcon_response_h response = NULL;
		iotcon_representation_h representation = NULL;
		iotcon_state_h state = NULL;
		iotcon_list_h list = NULL;

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

		ret = iotcon_list_create(IOTCON_TYPE_INT, &list);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_list_add_int(list, 1);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_list_destroy(list);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_list_add_int(list, 2);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_list_destroy(list);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_list_add_int(list, 10);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_list_destroy(list);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = itocon_state_set_list(state, "ids", list);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_list_destroy(list);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_representation_set_state(representation, state);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_list_destroy(list);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		...

		ret = iotcon_response_set_representation(response, IOTCON_INTERFACE_DEFAULT,
				representation);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_list_destroy(list);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		ret = iotcon_response_send(response);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_list_destroy(list);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(representation);
			iotcon_response_destroy(resopnse);
			return;
		}

		iotcon_list_destroy(list);
		iotcon_state_destroy(state);
		iotcon_representation_destroy(representation);
		iotcon_response_destroy(resopnse);
	}
	...
}
 * @endcode
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_LIST_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/iot.oic\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/tools/native-tools/manifest-text-editor#feature"><b>Feature Element</b>.</a>
 *
 * @{
 */

/**
 * @brief Creates a new list handle.
 *
 * @since_tizen 3.0
 *
 * @remarks You must destroy @a list by calling iotcon_list_destroy()
 * if @a list is no longer needed.
 *
 * @param[in] type The type of list
 * @param[out] list A newly allocated list handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_create(iotcon_type_e type, iotcon_list_h *list);

/**
 * @brief Destroys a list handle.
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
 * @brief Adds a new element integer value into the list at the given position.
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_add_int(iotcon_list_h list, int val, int pos);

/**
 * @brief Adds a new element boolean value into the list at the given position.
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_add_bool(iotcon_list_h list, bool val, int pos);

/**
 * @brief Adds a new element double value into the list at the given position.
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_add_double(iotcon_list_h list, double val, int pos);

/**
 * @brief Adds a new element string value into the list at the given position.
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_add_str(iotcon_list_h list, char *val, int pos);

/**
 * @brief Adds a new element byte string value into the list at the given position.
 * @details If @a pos is negative, or is larger than the number of elements in the list,
 * the new value is added on to the end of the list.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] val The new byte string value
 * @param[in] len The length of @a val
 * @param[in] pos The position to insert value
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_add_byte_str(iotcon_list_h list, unsigned char *val, int len, int pos);

/**
 * @brief Adds a new element list into the list at the given position.
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_add_list(iotcon_list_h list, iotcon_list_h val, int pos);

/**
 * @brief Adds a new element state value into the list at the given position.
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_INVALID_TYPE  Invalid type
 */
int iotcon_list_add_state(iotcon_list_h list, iotcon_state_h val, int pos);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
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
 * @remarks @a val must not be released using free().
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] val The string value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_str(iotcon_list_h list, int pos, char **val);

/**
 * @brief Gets the string value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @remarks @a val must not be released using free().
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] val The byte string value to get
 * @param[out] len The length of the @a val
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_byte_str(iotcon_list_h list, int pos, unsigned char **val,
		int *len);

/**
 * @brief Gets the list value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @remarks @a dest must not be released using iotcon_list_destroy().
 *
 * @param[in] src The list handle
 * @param[in] pos The position
 * @param[out] dest The list value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
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
 * @remarks @a state must not be released using iotcon_state_destroy().
 *
 * @param[in] list The list handle
 * @param[in] pos The position
 * @param[out] state The state value to get
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation errors
 */
int iotcon_list_get_nth_state(iotcon_list_h list, int pos, iotcon_state_h *state);

/**
 * @brief Removes the value at the given position.
 * @details Iterates over the list until it reaches the @a pos-1 position.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[in] pos The position to delete
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data available
 */
int iotcon_list_remove_nth(iotcon_list_h list, int pos);

/**
 * @brief Gets the type of the list.
 * @details It gets the data type of value related the @a key in @a state.
 * The data type could be one of #iotcon_type_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The list handle
 * @param[out] type The data type of list.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_list_get_type(iotcon_list_h list, iotcon_type_e *type);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 */
int iotcon_list_get_length(iotcon_list_h list, unsigned int *length);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_int().
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the integer value (0 being the first)
 * @param[in] value The integer value
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_list_foreach_int() will invoke this callback function.
 *
 * @see iotcon_list_foreach_int()
 */
typedef bool (*iotcon_list_int_cb)(int pos, int value, void *user_data);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_int_cb() will be called for each item.
 *
 * @see iotcon_list_int_cb()
 */
int iotcon_list_foreach_int(iotcon_list_h list, iotcon_list_int_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_bool().
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the boolean value (0 being the first)
 * @param[in] value The boolean value
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_list_foreach_bool() will invoke this callback function.
 *
 * @see iotcon_list_foreach_bool()
 */
typedef bool (*iotcon_list_bool_cb)(int pos, bool value, void *user_data);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
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
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_list_foreach_double() will invoke this callback function.
 *
 * @see iotcon_list_foreach_double()
 */
typedef bool (*iotcon_list_double_cb)(int pos, double value, void *user_data);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_double_cb() will be called for each item.
 *
 * @see iotcon_list_double_cb()
 */
int iotcon_list_foreach_double(iotcon_list_h list, iotcon_list_double_cb cb,
		void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_byte_str().
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the string value (0 being the first)
 * @param[in] value The byte string value
 * @param[in] len The length of @a value
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_list_foreach_byte_str() will invoke this callback function.
 *
 * @see iotcon_list_foreach_byte_str()
 */
typedef bool (*iotcon_list_byte_str_cb)(int pos, const unsigned char *value, int len,
		void *user_data);

/**
 * @brief Gets all string values of the given list by invoking the callback function.
 * @details iotcon_list_byte_str_cb() will be called for each child.
 *
 * @since_tizen 3.0
 *
 * @param[in] list The handle to the list
 * @param[in] cb The callback function to get each string value
 * @param[in] user_data The user data to be passed to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_byte_str_cb() will be called for each item.
 *
 * @see iotcon_list_byte_str_cb()
 */
int iotcon_list_foreach_byte_str(iotcon_list_h list, iotcon_list_byte_str_cb cb,
		void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_str().
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the string value (0 being the first)
 * @param[in] value The string value
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_list_foreach_str() will invoke this callback function.
 *
 * @see iotcon_list_foreach_str()
 */
typedef bool (*iotcon_list_str_cb)(int pos, const char *value, void *user_data);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_str_cb() will be called for each item.
 *
 * @see iotcon_list_str_cb()
 */
int iotcon_list_foreach_str(iotcon_list_h list, iotcon_list_str_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_list().
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the list value (0 being the first)
 * @param[in] value The list value
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_list_foreach_list() will invoke this callback function.
 *
 * @see iotcon_list_foreach_list()
 */
typedef bool (*iotcon_list_list_cb)(int pos, iotcon_list_h value, void *user_data);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @post iotcon_list_list_cb() will be called for each item.
 *
 * @see iotcon_list_list_cb()
 */
int iotcon_list_foreach_list(iotcon_list_h list, iotcon_list_list_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_list_foreach_state().
 *
 * @since_tizen 3.0
 *
 * @param[in] pos The number of the state value (0 being the first)
 * @param[in] value The state value
 * @param[in] user_data The user data to pass to the function
 *
 * @return true to continue with the next iteration of the loop,
 * otherwise false to break out of the loop. #IOTCON_FUNC_CONTINUE and #IOTCON_FUNC_STOP
 * are more friendly values for the return.
 *
 * @pre iotcon_list_foreach_state() will invoke this callback function.
 *
 * @see iotcon_list_foreach_state()
 */
typedef bool (*iotcon_list_state_cb)(int pos, iotcon_state_h value, void *user_data);

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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
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

#endif /* __IOT_CONNECTIVITY_STRUCT_LIST_H__ */
