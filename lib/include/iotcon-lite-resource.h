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
#ifndef __IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_H__
#define __IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_H__

#include <stdint.h>
#include <iotcon-types.h>

/**
 * @file iotcon-lite-resource.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE Lite Resource
 *
 * @brief IoTCon Lite Resource provides API to encapsulate resources.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE_OVERVIEW Overview
 * This API provides that the users manages resources without request handler.
 * When client request by CRUD functions, internal default request handler will be invoked.
 * The default request handler will create response and send to client automatically.
 * When updated attributes by iotcon_lite_resource_update_attributes(), changes will notify to observers.
 *
 * Example :
 * @code
#include <iotcon.h>
...
static iotcon_lite_resource_h _resource;

static bool _attributes_changed_cb(iotcon_lite_resource_h resource, iotcon_attributes_h attributes, void *user_data)
{
	return true;
}

static void _create_light_resource()
{
	int ret;
	iotcon_lite_resource_h resource = NULL;
	iotcon_resource_types_h resource_types = NULL;
	iotcon_attributes_h attributes = NULL;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret)
		return;

	ret = iotcon_resource_types_add(resource_types, "org.tizen.light");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_attributes_create(&attributes);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_attributes_add_bool(attributes, "power", true);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_attributes_destroy(attributes);
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_attributes_add_int(attributes, "brightness", 75);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_attributes_destroy(attributes);
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_lite_resource_create("/light/1", resource_types,
			IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE, attributes,
			_attributes_changed_cb, NULL, &resource);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_attributes_destroy(attributes);
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	iotcon_attributes_destroy(attributes);
	iotcon_resource_types_destroy(resource_types);

	_resource = resource;
}

static void _update_brightness(int brightness)
{
	int ret;
	iotcon_attributes_h attributes = NULL;
	iotcon_attributes_h attributes_clone = NULL;

	ret = iotcon_lite_resource_get_attributes(_resource, &attributes);
	if (IOTCON_ERROR_NONE != ret)
		return;

	ret = iotcon_attributes_clone(attributes, &attributes_clone);
	if (IOTCON_ERROR_NONE != ret)
		return;

	ret = iotcon_attributes_add_int(attributes_clone, "brightness", brightness);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_attributes_destroy(attributes_clone);
		return;
	}

	ret = iotcon_lite_resource_update_attributes(_resource, attributes_clone);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_attributes_destroy(attributes_clone);
		return;
	}

	iotcon_attributes_destroy(attributes_clone);
}

 * @endcode
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/iot.ocf\n
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
 * @param[in] attributes The attributes of the lite resource
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_lite_resource_create()
 *
 * @return true to accept post request, otherwise false to reject it.
 *
 * @see iotcon_lite_resource_create()
 */
typedef bool (*iotcon_lite_resource_post_request_cb)(iotcon_lite_resource_h resource,
		iotcon_attributes_h attributes, void *user_data);


/**
 * @brief Creates a lite resource handle and registers the resource in server.
 * @details Registers a resource specified by @a uri_path, @a res_types, @a attributes which have
 * @a properties in IoTCon server.\n
 * When client requests some operations, it send a response to client, automatically.\n
 * The @a policies can contain multiple policies like
 * IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks @a uri_path length must be less than 128.\n
 * You must destroy @a resource_handle by calling iotcon_lite_resource_destroy()
 * if @a remote_handle is no longer needed.
 *
 * @param[in] uri_path The URI path of the resource
 * @param[in] res_types The list of type of the resource
 * @param[in] policies The policies of the resource\n Set of #iotcon_resource_policy_e
 * @param[in] attributes The attributes handle to set
 * @param[in] cb The callback function to add into callback list
 * @param[in] user_data The user data to pass to the callback function
 * @param[out] resource_handle The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  IoTivity errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 * @retval #IOTCON_ERROR_NOT_INITIALIZED Not initialized
 *
 * @pre iotcon_initialize() should be called to initialize.
 *
 * @see iotcon_lite_resource_destroy()
 */
int iotcon_lite_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		uint8_t policies,
		iotcon_attributes_h attributes,
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
 * @retval #IOTCON_ERROR_NOT_INITIALIZED Not initialized
 *
 * @pre iotcon_initialize() should be called to initialize.
 *
 * @see iotcon_lite_resource_create()
 */
int iotcon_lite_resource_destroy(iotcon_lite_resource_h resource);

/**
 * @brief Updates attributes into the lite resource handle.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the lite resource
 * @param[in] attributes The attributes handle to update
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 * @retval #IOTCON_ERROR_NOT_INITIALIZED Not initialized
 *
 * @pre iotcon_initialize() should be called to initialize.
 *
 * @see iotcon_lite_resource_get_attributes()
 */
int iotcon_lite_resource_update_attributes(iotcon_lite_resource_h resource,
		iotcon_attributes_h attributes);

/**
 * @brief Gets attributes from the lite resource handle.
 *
 * @since_tizen 3.0
 *
 * @remarks @a attributes must not be released using iotcon_attributes_destroy().
 *
 * @param[in] resource The handle of the lite resource
 * @param[out] attributes The attributes handle of the lite resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_lite_resource_update_attributes()
 */
int iotcon_lite_resource_get_attributes(iotcon_lite_resource_h resource,
		iotcon_attributes_h *attributes);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_H__ */
