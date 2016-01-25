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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file iotcon-internal.h
 */

/**
 * @brief Enumeration for mode of iotcon service.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_SERVICE_IP, /**< IP(IPv4, IPv6) mode */
	IOTCON_SERVICE_BT, /**< Bluetooth (EDR, LE) mode */
} iotcon_service_mode_e;

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
int iotcon_connect_for_service_mode(iotcon_service_mode_e mode);

/**
 * @brief Starts presence of a server.
 * @details Use this function to send server's announcements to clients.\n
 * Server can call this function when online for the first time or come back from offline to online.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/network.get
 * @privilege %http://tizen.org/privilege/d2d.datasharing
 *
 * @remarks If @a time_to_live is 0, server will set default value as 60 seconds.\n
 * If @a time_to_live is very big, server will set maximum value as (60 * 60 * 24) seconds.
 * (24 hours)
 *
 * @param[in] time_to_live The interval of announcing presence in seconds.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_stop_presence()
 * @see iotcon_add_presence_cb()
 * @see iotcon_remove_presence_cb()
 */
int iotcon_start_presence(unsigned int time_to_live);

/**
 * @brief Stops presence of a server.
 * @details Use this function to stop sending server's announcements to clients.
 * Server can call this function when terminating, entering to offline or out of network.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/network.get
 * @privilege %http://tizen.org/privilege/d2d.datasharing
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_start_presence()
 * @see iotcon_add_presence_cb()
 * @see iotcon_remove_presence_cb()
 */
int iotcon_stop_presence(void);

/**
 * @brief Gets the time interval of monitoring & caching API of remote resource.
 * @details This API get the time interval of iotcon_remote_resource_start_monitoring(),
 * and iotcon_remote_resource_start_caching().\n
 * The functions operate GET method, every saved time interval.
 * Default time interval is 10 seconds.
 *
 * @since_tizen 3.0
 *
 * @param[out] time_interval Seconds for time interval
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see iotcon_remote_resource_set_time_interval()
 * @see iotcon_remote_resource_start_monitoring()
 * @see iotcon_remote_resource_start_caching()
 */
int iotcon_remote_resource_get_time_interval(int *time_interval);

/**
 * @brief Sets the time interval of monitoring & caching API of remote resource.
 * @details This API set the time interval of iotcon_remote_resource_start_monitoring(),
 * and iotcon_remote_resource_start_caching().
 *
 * @since_tizen 3.0
 *
 * @param[in] time_interval Seconds for time interval (must be in range from 1 to 3600)
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @see iotcon_remote_resource_get_time_interval()
 * @see iotcon_remote_resource_start_monitoring()
 * @see iotcon_remote_resource_start_caching()
 */
int iotcon_remote_resource_set_time_interval(int time_interval);


#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_H__ */
