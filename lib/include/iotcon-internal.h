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

#include <iotcon.h>

/**
 * @file iotcon-internal.h
 */

/**
 * @brief Bluetooth Enhanced Data Rate Connectivity
 *
 * @since_tizen 3.0
 */
#define IOTCON_CONNECTIVITY_BT_EDR 10000

/**
 * @brief Bluetooth Low Energy Connectivity
 *
 * @since_tizen 3.0
 */
#define IOTCON_CONNECTIVITY_BT_LE 10001

/**
 * @brief Bluetooth Enhanced Data Rate Connectivity & Low Energy Connectivity
 *
 * @since_tizen 3.0
 */
#define IOTCON_CONNECTIVITY_BT_ALL 10002

/**
 * @brief Enumeration for mode of iotcon service.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_SERVICE_IP = (1 << 0), /**< IP(IPv4, IPv6) mode */
	IOTCON_SERVICE_BT = (1 << 1), /**< Bluetooth (EDR, LE) mode */
	IOTCON_SERVICE_BOTH = IOTCON_SERVICE_IP | IOTCON_SERVICE_BT, /**< Both mode */
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
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_DBUS Dbus error
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

/**
 * @brief Creates a resource handle and registers the resource in server
 * @details Registers a resource specified by @a uri_path, @a res_types, @a ifaces which have
 * @a properties in Iotcon server.\n
 * When client find the registered resource, iotcon_request_handler_cb() will be called automatically.\n
 * @a uri_path format would be relative URI path like '/a/light'\n
 * @a res_types is a list of resource types. Create a iotcon_resource_types_h handle and
 * add types string to it.\n
 * @a ifaces is a list of resource interfaces. Create a iotcon_resource_interfaces_h handle and
 * add interfaces string to it.\n
 * @a properties also can contain multiple properties like
 * IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE.\n
 * iotcon_request_handler_cb() will be called when receive CRUD request to the registered
 * resource.
 * @a connectivity_type is either #IOTCON_CONNECTIVITY_ALL or #IOTCON_CONNECTIVITY_BT_ALL.\n
 * #IOTCON_CONNECTIVITY_ALL means IP Version 4 and IP Version 6.
 * #IOTCON_CONNECTIVITY_BT_ALL means Bluetooth Enhanced Data Rate and Bluetooth Low Energy.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/network.get
 * @privilege %http://tizen.org/privilege/d2d.datasharing
 *
 * @remarks @a uri_path length must be less than or equal 36.\n
 * You must destroy @a resource by calling iotcon_resource_destroy()
 * if @a resource is no longer needed.
 *
 * @param[in] uri_path The URI path of the resource
 * @param[in] res_types The list of type of the resource
 * @param[in] ifaces The list of interface of the resource
 * @param[in] properties The properties of the resource\n Set of #iotcon_resource_property_e
 * @param[in] cb The request handler callback function
 * @param[in] user_data The user data to pass to the callback function
 * @param[in] connectivity_type The connectivity type related with the resource
 * @param[out] resource_handle The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the resource receive CRUD request, iotcon_request_handler_cb() will be called.
 *
 * @see iotcon_resource_destroy()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_set_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_create2(const char *uri_path,
		iotcon_resource_types_h res_types,
		iotcon_resource_interfaces_h ifaces,
		int properties,
		iotcon_request_handler_cb cb,
		void *user_data,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_resource_h *resource_handle);

/**
 * @brief Creates a lite resource handle and registers the resource in server
 * @details Registers a resource specified by @a uri_path, @a res_types, @a state which have
 * @a properties in Iotcon server.\n
 * When client requests some operations, it send a response to client, automatically.\n
 * The @a properties can contain multiple properties like
 * IOTCON_RESOURCE_DISCOVERABLE | IOTCON_RESOURCE_OBSERVABLE.\n
 * @a connectivity_type is either #IOTCON_CONNECTIVITY_ALL or #IOTCON_CONNECTIVITY_BT_ALL.\n
 * #IOTCON_CONNECTIVITY_ALL means IP Version 4 and IP Version 6.
 * #IOTCON_CONNECTIVITY_BT_ALL means Bluetooth Enhanced Data Rate and Bluetooth Low Energy.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/network.get
 * @privilege %http://tizen.org/privilege/d2d.datasharing
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
 * @param[in] connectivity_type The connectivity type related with the resource
 * @param[out] resource_handle The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_lite_resource_destroy()
 */
int iotcon_lite_resource_create2(const char *uri_path,
		iotcon_resource_types_h res_types,
		int properties,
		iotcon_state_h state,
		iotcon_lite_resource_post_request_cb cb,
		void *user_data,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_lite_resource_h *resource_handle);

#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_H__ */
