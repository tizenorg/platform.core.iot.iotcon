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
#ifndef __IOT_CONNECTIVITY_MANAGER_COMMON_RESPONSE_H__
#define __IOT_CONNECTIVITY_MANAGER_COMMON_RESPONSE_H__

#include <iotcon-constant.h>

/**
 * @file iotcon-response.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_COMMON_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_COMMON_RESPONSE_MODULE Response
 *
 * @brief Iotcon Response provides API to manage response.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_RESPONSE_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @{
 */

/**
 * @brief Creates a response handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] request The handle of received request handle
 * @param[out] response Generated response handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_response_destroy()
 * @see iotcon_response_set()
 */
int iotcon_response_create(iotcon_request_h request, iotcon_response_h *response);

/**
 * @brief Free a response handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 *
 * @see iotcon_response_create()
 * @see iotcon_response_set()
 */
void iotcon_response_destroy(iotcon_response_h resp);


/**
 * @brief Gets header options of the response
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 * @param[out] options The handle of the header options
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data
 *
 * @see iotcon_response_get_representation()
 * @see iotcon_response_get_result()
 */
int iotcon_response_get_options(iotcon_response_h resp, iotcon_options_h *options);

/**
 * @brief Gets representation of the response
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 * @param[out] repr The handle of the representation
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_NO_DATA  No data
 *
 * @see iotcon_response_get_options()
 * @see iotcon_response_get_result()
 */
int iotcon_response_get_representation(iotcon_response_h resp, iotcon_representation_h *repr);

/**
 * @brief Gets result of the response
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 * @param[out] result The result of the response
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_response_get_options()
 * @see iotcon_response_get_representation()
 */
int iotcon_response_get_result(iotcon_response_h resp, int *result);


/**
 * @brief Sets result into the response
 * @details The @a result could be one of #iotcon_response_result_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 * @param[in] result The result to set
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_response_create()
 * @see iotcon_response_destroy()
 */
int iotcon_response_set_result(iotcon_response_h resp, iotcon_response_result_e result);

/**
 * @brief Sets representation into the response
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 * @param[in] repr The representation of the response
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_response_create()
 * @see iotcon_response_destroy()
 */
int iotcon_response_set_representation(iotcon_response_h resp,
		iotcon_interface_e iface, iotcon_representation_h repr);

/**
 * @brief Sets header options into the response
 *
 * @since_tizen 3.0
 *
 * @param[in] resp The handle of the response
 * @param[in] options The header options of the response
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_response_create()
 * @see iotcon_response_destroy()
 */
int iotcon_response_set_header_options(iotcon_response_h resp, iotcon_options_h options);

/**
 * @brief Send response for incoming request.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resp The handle of the response to send
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 */
int iotcon_response_send(iotcon_response_h resp);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_COMMON_RESPONSE_H__ */
