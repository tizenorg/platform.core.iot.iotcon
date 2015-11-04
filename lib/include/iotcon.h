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
#ifndef __IOT_CONNECTIVITY_MANAGER_H__
#define __IOT_CONNECTIVITY_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <tizen_type.h>

#include <iotcon-constant.h>
#include <iotcon-errors.h>
#include <iotcon-struct.h>
#include <iotcon-server.h>
#include <iotcon-client.h>

/**
 * @file iotcon.h
 */

/**
 *
 * @addtogroup CAPI_IOT_CONNECTIVITY_MODULE
 *
 * @{
 */

/**
 * @brief Connects to the iotcon service.
 * @details Call this function to start Iotcon.
 *
 * @since_tizen 3.0
 *
 * @remarks You must free all resources of the Iotcon by calling iotcon_disconnect()
 * if Iotcon API is no longer needed.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #IOTCON_ERROR_NONE Successful
 * @retval  #IOTCON_ERROR_DBUS Dbus error
 *
 * @see iotcon_disconnect()
 */
int iotcon_connect(void);

/**
 * @brief Disconnects from the iotcon service.
 * @details Frees the resources allocated to Iotcon.
 *
 * @since_tizen 3.0
 *
 * @remarks This function must be called if Iotcon API is no longer needed.
 *
 * @return void
 *
 * @see iotcon_connect()
 */
void iotcon_disconnect(void);

/**
 * @brief Gets timeout of asynchronous APIs.
 *
 * @since_tizen 3.0
 *
 * @param[out] timeout_seconds Seconds for timeout
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @pre iotcon_connect() should be called to connect a connection to the iotcon.
 *
 * @see iotcon_get_device_info()
 * @see iotcon_get_platform_info()
 * @see iotcon_get_tizen_info()
 * @see iotcon_find_resource()
 * @see iotcon_remote_resource_get()
 * @see iotcon_remote_resource_put()
 * @see iotcon_remote_resource_post()
 * @see iotcon_remote_resource_delete()
 */
int iotcon_get_timeout(int *timeout_seconds);


/**
 * @brief Set timeout of asynchronous APIs.
 * @details Default timeout is 10 seconds.
 * Maximum timeout is 60 seconds.
 *
 * @since_tizen 3.0
 *
 * @param[in] timeout_seconds Seconds for timeout
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #IOTCON_ERROR_NONE Successful
 * @retval  #IOTCON_ERROR_DBUS Dbus error
 *
 * @pre iotcon_connect() should be called to connect a connection to the iotcon.
 *
 * @see iotcon_request_device_info()
 * @see iotcon_request_platform_info()
 * @see iotcon_request_tizen_info()
 * @see iotcon_find_resource()
 * @see iotcon_remote_resource_get()
 * @see iotcon_remote_resource_put()
 * @see iotcon_remote_resource_post()
 * @see iotcon_remote_resource_delete()
 */
int iotcon_set_timeout(int timeout_seconds);

/**
 * @brief Specifies the type of function passed to iotcon_add_connection_changed_cb() and
 * iotcon_remove_connection_changed_cb().
 *
 * @since_tizen 3.0
 *
 * @param[in] is_connected The status of connection
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_add_connection_changed_cb()\n
 * The callback must be unregistered using iotcon_remove_connection_changed_cb()\n
 *
 * @see iotcon_add_connection_changed_cb()
 * @see iotcon_remove_connection_changed_cb()
 */
typedef void (*iotcon_connection_changed_cb)(bool is_connected, void *user_data);

/**
 * @brief Adds a callback to Iotcon
 * @details When Iotcon connection status is changed, registered callbacks will be called in turn.
 *
 * @since_tizen 3.0
 *
 * @param[in] cb The callback function to add into callback list
 * @param[in] user_data The user data to pass to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_ALREADY  Already done
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 *
 * @see iotcon_remove_connection_changed_cb()
 * @see iotcon_connection_changed_cb()
 */
int iotcon_add_connection_changed_cb(iotcon_connection_changed_cb cb, void *user_data);

/**
 * @brief Removes the callback from the callback list.
 * @details Finds out the callback passing to parameter from registered callbacks, then remove it.
 *
 * @since_tizen 3.0
 *
 * @param[in] cb The callback function to remove from callback list
 * @param[in] user_data The user data to pass to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_add_connection_changed_cb()
 * @see iotcon_connection_changed_cb()
 */
int iotcon_remove_connection_changed_cb(iotcon_connection_changed_cb cb, void *user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_MANAGER_H__ */
