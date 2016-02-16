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

#include <iotcon-types.h>
#include <iotcon-errors.h>
#include <iotcon-server.h>
#include <iotcon-client.h>

/**
 * @file iotcon.h
 */

/**
 *
 * @addtogroup CAPI_IOT_CONNECTIVITY_MODULE
 *
 * @section CAPI_IOT_CONNECTIVITY_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 * - http://tizen.org/feature/iot.oic\n
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
 * @brief Connects to the iotcon service.
 * @details Call this function to start Iotcon.
 *
 * @since_tizen 3.0
 *
 * @param[in] mode Service mode of iotcon
 *
 * @remarks You must free all resources of the Iotcon by calling iotcon_disconnect()
 * if Iotcon API is no longer needed.
 *
 * @return	0 on success, otherwise a negative error value.
 * @retval	#IOTCON_ERROR_NONE Successful
 * @retval	#IOTCON_ERROR_DBUS Dbus error
 *
 * @see iotcon_disconnect()
 * @see iotcon_add_connection_changed_cb()
 * @see iotcon_remove_connection_changed_cb()
 */
int iotcon_connect2(iotcon_service_mode_e mode);

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
 * @see iotcon_add_connection_changed_cb()
 * @see iotcon_remove_connection_changed_cb()
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
 * @see iotcon_add_connection_changed_cb()
 * @see iotcon_remove_connection_changed_cb()
 */
void iotcon_disconnect(void);

/**
 * @brief Gets the timeout seconds of asynchronous API.
 * @details This API get the timeout of iotcon_get_device_info(),
 * iotcon_get_platform_info(), iotcon_find_resource(),
 * iotcon_remote_resource_get(), iotcon_remote_resource_put(),
 * iotcon_remote_resource_post() and iotcon_remote_resource_delete().
 *
 * @since_tizen 3.0
 *
 * @param[out] timeout_seconds Seconds for timeout
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @pre iotcon_connect() should be called to connect a connection to the iotcon.
 *
 * @see iotcon_set_timeout()
 */
int iotcon_get_timeout(int *timeout_seconds);

/**
 * @brief Sets the timeout seconds of asynchrous APIs.
 * @details This API set the timeout of iotcon_get_device_info(),
 * iotcon_get_platform_info(), iotcon_find_resource(),
 * iotcon_remote_resource_get(), iotcon_remote_resource_put(),
 * iotcon_remote_resource_post() and iotcon_remote_resource_delete().\n
 * Default timeout interval value is 30.
 *
 * @since_tizen 3.0
 *
 * @param[in] timeout_seconds Seconds for timeout (must be in range from 1 to 3600)
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #IOTCON_ERROR_DBUS Dbus error
 *
 * @pre iotcon_connect() should be called to connect a connection to the iotcon.
 *
 * @see iotcon_get_timeout()
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
 * @see iotcon_connect()
 * @see iotcon_disconnect()
 * @see iotcon_add_connection_changed_cb()
 * @see iotcon_remove_connection_changed_cb()
 */
typedef void (*iotcon_connection_changed_cb)(bool is_connected, void *user_data);

/**
 * @brief Adds a callback to Iotcon
 * @details When Iotcon connection status is changed with unexpected reason, registered
 * callbacks will be called in turn. The handles(e.g. iotcon_resource_h,
 * iotcon_remote_resource_h, ...) are validate no more after broken connection.
 * The handles must creates again when connection recovered.
 *
 * @since_tizen 3.0
 *
 * @param[in] cb The callback function to add into callback list
 * @param[in] user_data The user data to pass to the callback function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_ALREADY  Already done
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 *
 * @pre iotcon_connect() should be called to connect a connection to the iotcon.
 *
 * @see iotcon_connect()
 * @see iotcon_disconnect()
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
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @pre iotcon_connect() should be called to connect a connection to the iotcon.
 *
 * @see iotcon_connect()
 * @see iotcon_disconnect()
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
