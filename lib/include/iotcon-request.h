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
#ifndef __IOT_CONNECTIVITY_MANAGER_SERVER_REQUEST_H__
#define __IOT_CONNECTIVITY_MANAGER_SERVER_REQUEST_H__

#include <iotcon-types.h>

/**
 * @file iotcon-request.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_REQUEST_MODULE Request
 *
 * @brief Iotcon Request provides API to manage client's request.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_REQUEST_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @{
 */

/**
 * @brief Gets host address of the request
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
 * @brief Gets connectivity type of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] connectivity_type The connectivity type of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
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
 * @brief Gets an representation of the request
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
 * @brief Gets type of the request
 * @details @a type could be one of the #iotcon_request_type_e
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] type The types of the request.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
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
 * @brief Gets options of the request
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
 * @brief Gets query of the request
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
 * @brief Gets observation action of the request
 *
 * @since_tizen 3.0
 * @details The @a observe_type could be one of #iotcon_observe_type_e.
 *
 * @param[in] request The handle of the request
 * @param[out] observe_type The observation type of the request
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
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
 * @brief Gets observation id of the request
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of the request
 * @param[out] observe_id The id of the observer
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
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

#endif /* __IOT_CONNECTIVITY_MANAGER_SERVER_REQUEST_H__ */
