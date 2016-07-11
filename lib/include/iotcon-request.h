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
#ifndef __IOT_CONNECTIVITY_SERVER_REQUEST_H__
#define __IOT_CONNECTIVITY_SERVER_REQUEST_H__

#include <iotcon-types.h>

/**
 * @file iotcon-request.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_REQUEST_MODULE Request
 *
 * @brief IoTCon Request provides API to manage client's request.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_REQUEST_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_REQUEST_MODULE_OVERVIEW Overview
 * The iotcon request API provides methods for managing request handle.
 *
 * Example :
 * @code
#include <iotcon.h>
static void _request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
{
	int ret, observe_id;
	iotcon_request_type_e type;
	iotcon_observe_type_e observe_type;
	iotcon_options_h options = NULL;
	iotcon_query_h query = NULL;
	iotcon_representation_h repr = NULL;

	ret = iotcon_request_get_options(request, &options);
	if (IOTCON_ERROR_NONE == ret && options) {
		// handle options
		...
	}

	ret = iotcon_request_get_query(request, &query);
	if (IOTCON_ERROR_NONE == ret && query) {
		// handle query
		...
	}

	ret = iotcon_request_get_request_type(request, &type);
	if (IOTCON_ERROR_NONE != ret)
		return;

	if (IOTCON_REQUEST_GET == type) {
		// handle get
		...
	}
	if (IOTCON_REQUEST_PUT == type) {
		// handle put
		ret = iotcon_request_get_representation(request, &repr);
		if (IOTCON_ERROR_NONE != ret)
			return;
		...
	}
	if (IOTCON_REQUEST_POST == type) {
		// handle post
		ret = iotcon_request_get_representation(request, &repr);
		if (IOTCON_ERROR_NONE != ret)
			return;
		...
	}
	if (IOTCON_REQUEST_DELETE == type) {
		// handle delete
		ret = iotcon_request_get_representation(request, &repr);
		if (IOTCON_ERROR_NONE != ret)
			return;
		...
	}

	ret = iotcon_request_get_observe_type(request, &observe_type);
	if (IOTCON_ERROR_NONE != ret)
		return;

	if (IOTCON_OBSERVE_REGISTER == observe_type) {
		ret = iotcon_request_get_observe_id(request, &observe_id);
		if (IOTCON_ERROR_NONE != ret)
			return;
		// handle register observe
		...
	} else if (IOTCON_OBSERVE_DEREGISTER == observe_type) {
		ret = iotcon_request_get_observe_id(request, &observe_id);
		if (IOTCON_ERROR_NONE != ret)
			return;
		// handle deregister observe
		...
	}
	...
}
 * @endcode
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_REQUEST_MODULE_FEATURE Related Features
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
 * @brief Gets host address of the request.
 *
 * @since_tizen 3.0
 *
 * @remarks @a host_address must not be released using free().
 *
 * @param[in] request The handle of the request
 * @param[out] host_address The host address of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_connectivity_type()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_request_type()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observe_type()
 * @see iotcon_request_get_observe_id()
 */
int iotcon_request_get_host_address(iotcon_request_h request,
		char **host_address);

/**
 * @brief Gets connectivity type of the request.
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] connectivity_type The connectivity type of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_host_address()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_request_type()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observe_type()
 * @see iotcon_request_get_observe_id()
 */
int iotcon_request_get_connectivity_type(iotcon_request_h request,
		iotcon_connectivity_type_e *connectivity_type);

/**
 * @brief Gets an representation of the request.
 *
 * @since_tizen 3.0
 *
 * @remarks @a repr must not be released using iotcon_representation_destroy().
 *
 * @param[in] request The handle of the request
 * @param[out] repr The representation of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_host_address()
 * @see iotcon_request_get_connectivity_type()
 * @see iotcon_request_get_request_type()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observe_type()
 * @see iotcon_request_get_observe_id()
 */
int iotcon_request_get_representation(iotcon_request_h request,
		iotcon_representation_h *repr);

/**
 * @brief Gets type of the request.
 * @details @a type could be one of the #iotcon_request_type_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] type The types of the request.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_host_address()
 * @see iotcon_request_get_connectivity_type()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observe_type()
 * @see iotcon_request_get_observe_id()
 */
int iotcon_request_get_request_type(iotcon_request_h request, iotcon_request_type_e *type);

/**
 * @brief Gets options of the request.
 *
 * @since_tizen 3.0
 *
 * @remarks @a options must not be released using iotcon_options_destroy().
 *
 * @param[in] request The handle of the request
 * @param[out] options The options of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_host_address()
 * @see iotcon_request_get_connectivity_type()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_request_type()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observe_type()
 * @see iotcon_request_get_observe_id()
 */
int iotcon_request_get_options(iotcon_request_h request, iotcon_options_h *options);

/**
 * @brief Gets query of the request.
 *
 * @since_tizen 3.0
 *
 * @remarks @a query must not be released using iotcon_query_destroy().
 *
 * @param[in] request The handle of the request
 * @param[out] query The query of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_host_address()
 * @see iotcon_request_get_connectivity_type()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_request_type()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_observe_type()
 * @see iotcon_request_get_observe_id()
 */
int iotcon_request_get_query(iotcon_request_h request, iotcon_query_h *query);

/**
 * @brief Gets observation action of the request.
 *
 * @since_tizen 3.0
 * @details The @a observe_type could be one of #iotcon_observe_type_e.
 *
 * @param[in] request The handle of the request
 * @param[out] observe_type The observation type of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_host_address()
 * @see iotcon_request_get_connectivity_type()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_request_type()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observe_id()
 */
int iotcon_request_get_observe_type(iotcon_request_h request,
		iotcon_observe_type_e *observe_type);

/**
 * @brief Gets observation id of the request.
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] observe_id The id of the observer
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_request_get_host_address()
 * @see iotcon_request_get_connectivity_type()
 * @see iotcon_request_get_representation()
 * @see iotcon_request_get_request_type()
 * @see iotcon_request_get_options()
 * @see iotcon_request_get_query()
 * @see iotcon_request_get_observe_type()
 */
int iotcon_request_get_observe_id(iotcon_request_h request, int *observe_id);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_SERVER_REQUEST_H__ */
