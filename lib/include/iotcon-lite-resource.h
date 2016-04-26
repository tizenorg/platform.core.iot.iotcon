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
#ifndef __IOT_CONNECTIVITY_MANAGER_SERVER_LITE_RESOURCE_H__
#define __IOT_CONNECTIVITY_MANAGER_SERVER_LITE_RESOURCE_H__

#include <iotcon-types.h>

/**
 * @file iotcon-lite-resource.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE Lite Resource
 *
 * @brief Iotcon Lite Resource provides API to encapsulate resources.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE_OVERVIEW Overview
 * This API provides that the users manages resources without request handler.
 * When client request by CRUD functions, internal default request handler will be invoked.
 * The default request handler will create response and send to client automatically.
 * When updated state by iotcon_lite_update_state(), changes will notify to observers.
 *
 * Example :
 * @code
#include <iotcon.h>
...
static iotcon_lite_resource_h _resource;

static bool _state_changed_cb(iotcon_lite_resource_h resource, iotcon_state_h state, void *user_data)
{
	return true;
}

static void _create_light_resource()
{
	int ret;
	iotcon_lite_resource_h resource = NULL;
	iotcon_resource_types_h resource_types = NULL;
	iotcon_state_h state = NULL;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret)
		return;

	ret = iotcon_resource_types_add(resource_types, "org.tizen.light");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_state_create(&state);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_state_add_bool(state, "power", true);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_state_destroy(state);
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_state_add_int(state, "brightness", 75);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_state_destroy(state);
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_lite_resource_create("/light/1", resource_types,
			IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE, state,
			_state_changed_cb, NULL, &resource);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_state_destroy(state);
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	iotcon_state_destroy(state);
	iotcon_resource_types_destroy(resource_types);

	_resource = resource;
}

static void _update_brightness(int brightness)
{
	int ret;
	iotcon_state_h state = NULL;
	iotcon_state_h state_clone = NULL;

	ret = iotcon_lite_resource_get_state(_resource, &state);
	if (IOTCON_ERROR_NONE != ret)
		return;

	ret = iotcon_state_clone(state, &state_clone);
	if (IOTCON_ERROR_NONE != ret)
		return;

	ret = iotcon_state_add_int(state_clone, "brightness", brightness);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_state_destroy(state_clone);
		return;
	}

	ret = iotcon_lite_resource_update_state(_resource, state_clone);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_state_destroy(state_clone);
		return;
	}

	iotcon_state_destroy(state_clone);
}

 * @endcode
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE_FEATURE Related Features
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
 * @brief Specifies the type of function passed to iotcon_lite_resource_create().
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the lite resource
 * @param[in] state The state of the lite resource
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_lite_resource_create()
 *
 * @return true to accept post request, otherwise false to reject it.
 *
 * @see iotcon_lite_resource_create()
 */
typedef bool (*iotcon_lite_resource_post_request_cb)(iotcon_lite_resource_h resource,
		iotcon_state_h state, void *user_data);


/**
 * @brief Creates a lite resource handle and registers the resource in server.
 * @details Registers a resource specified by @a uri_path, @a res_types, @a state which have
 * @a properties in Iotcon server.\n
 * When client requests some operations, it send a response to client, automatically.\n
 * The @a properties can contain multiple properties like
 * IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks @a uri_path length must be less than or equal 36.\n
 * You must destroy @a resource_handle by calling iotcon_lite_resource_destroy()
 * if @a remote_handle is no longer needed.
 *
 * @param[in] uri_path The URI path of the resource
 * @param[in] res_types The list of type of the resource
 * @param[in] properties The property of the resource\n Set of #iotcon_resource_property_e
 * @param[in] state The state handle to set
 * @param[in] cb The callback function to add into callback list
 * @param[in] user_data The user data to pass to the callback function
 * @param[out] resource_handle The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_lite_resource_destroy()
 */
int iotcon_lite_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		int properties,
		iotcon_state_h state,
		iotcon_lite_resource_post_request_cb cb,
		void *user_data,
		iotcon_lite_resource_h *resource_handle);

/**
 * @brief Destroys the resource and releases its data.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks When a normal variable is used, there are only permission denied error.
 * If the errors of this API are not handled, then you must check an application have
 * the privileges for the API.
 *
 * @param[in] resource The handle of the lite resource to be unregistered
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_lite_resource_create()
 */
int iotcon_lite_resource_destroy(iotcon_lite_resource_h resource);

/**
 * @brief Updates state into the lite resource handle.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the lite resource
 * @param[in] state The state handle to update
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_lite_resource_get_state()
 */
int iotcon_lite_resource_update_state(iotcon_lite_resource_h resource,
		iotcon_state_h state);

/**
 * @brief Gets state from the lite resource handle.
 *
 * @since_tizen 3.0
 *
 * @remarks @a state must not be released using iotcon_state_destroy().
 *
 * @param[in] resource The handle of the lite resource
 * @param[out] state The state handle of the lite resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_lite_resource_update_state()
 */
int iotcon_lite_resource_get_state(iotcon_lite_resource_h resource,
		iotcon_state_h *state);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_SERVER_LITE_RESOURCE_H__ */
