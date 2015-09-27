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

#include <stdint.h>
#include <stdbool.h>
#include <iotcon-errors.h>
#include <iotcon-struct.h>
#include <iotcon-constant.h>
#include <iotcon-representation.h>

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
 * @brief Opens Iotcon.
 * @details Call this function to start Iotcon.
 *
 * @since_tizen 3.0
 *
 * @remarks You must free all resources of the Iotcon by calling iotcon_close()
 * if Iotcon API is no longer needed.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval  #IOTCON_ERROR_NONE Successful
 * @retval  #IOTCON_ERROR_DBUS Dbus error
 *
 * @see iotcon_close()
 */
int iotcon_open(void);

/**
 * @brief Closes Iotcon.
 * @details Frees the resources allocated to Iotcon.
 *
 * @since_tizen 3.0
 *
 * @remarks This function must be called if Iotcon API is no longer needed.
 *
 * @return void
 *
 * @see iotcon_open()
 */
void iotcon_close(void);

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
 * @brief Specifies the type of function passed to iotcon_register_resource() and
 * iotcon_resource_bind_request_handler()
 * @details Called when server receive request from the client.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The resource requested
 * @param[in] request The request from client
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_register_resource()
 *
 * @see iotcon_register_resource()
 * @see iotcon_resource_bind_request_handler()
 */
typedef void (*iotcon_request_handler_cb)(iotcon_resource_h resource,
		iotcon_request_h request, void *user_data);

/**
 * @brief Registers a resource in server
 * @details Registers a resource specified by @a uri_path, @a res_types, @a ifaces which have
 * @a properties in Iotcon server.\n
 * When client find the registered resource, iotcon_request_handler_cb() will be called automatically.\n
 * @a uri_path format would be relative URI path like '/a/light'\n
 * @a res_types is a list of resource types. Create a iotcon_resource_types_h handle and
 * add types string to it.\n
 * @a ifaces can contain multiple interfaces like
 * IOTCON_INTERFACE_LINK | IOTCON_INTERFACE_BATCH.\n
 * @a properties also can contain multiple properties like
 * IOTCON_ACTIVE | IOTCON_DISCOVERABLE.\n
 * iotcon_request_handler_cb() will be called when receive CRUD request to the registered
 * resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks @a uri_path length must be less than or equal 36.\n
 * You must unregister resource by calling iotcon_unregister_resource()
 * if resource is no longer needed.
 *
 * @param[in] uri_path The URI path of the resource.
 * @param[in] res_types The list of type of the resource.
 * @param[in] ifaces The interfaces of the resource.
 * @param[in] properties The property of the resource.
 * @param[in] cb The request handler callback function
 * @param[in] user_data The user data to pass to the callback function
 * @param[out] resource_handle The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the resource receive CRUD request, iotcon_request_handler_cb() will be called.
 *
 * @see iotcon_unregister_resource()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_bind_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_register_resource(const char *uri_path,
		iotcon_resource_types_h res_types,
		int ifaces,
		uint8_t properties,
		iotcon_request_handler_cb cb,
		void *user_data,
		iotcon_resource_h *resource_handle);

/**
 * @brief Unregisters a resource and releases its data.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks When a normal variable is used, there are only dbus error and permission\n
 * denied error. If the errors of this API are not handled, then you must check\n
 * whether dbus is running and an application have the privileges for the API.
 *
 * @param[in] resource_handle The handle of the resource to be unregistered
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_register_resource()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_bind_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_unregister_resource(iotcon_resource_h resource_handle);

/**
 * @brief Binds an interface to the resource
 *
 * @details The @a action could be one of #iotcon_interface_e.
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks Set only one interface to @a iface. If not, @a iface will be ignored.
 *
 * @param[in] resource The handle of the resource
 * @param[in] iface The interface to be bound to the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_register_resource()
 * @see iotcon_unregister_resource()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_bind_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_bind_interface(iotcon_resource_h resource, int iface);

/**
 * @brief Binds a type to the resource
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource_handle The handle of the resource
 * @param[in] resource_type The type to be bound to the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_register_resource()
 * @see iotcon_unregister_resource()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_bind_type(iotcon_resource_h resource_handle,
		const char *resource_type);

/**
 * @brief Binds a request handler to the resource
 * @details When the resource receive CRUD request, iotcon_request_handler_cb() will be
 * called.
 *
 * @since_tizen 3.0
 *
 * @remarks Registered callback function will be replaced with the new @a cb.\n
 *
 * @param[in] resource The handle of the resource
 * @param[in] cb The request handler to be bound to the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_register_resource()
 * @see iotcon_unregister_resource()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_bind_request_handler(iotcon_resource_h resource,
		iotcon_request_handler_cb cb);

/**
 * @brief Binds a child resource into the parent resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] parent The handle of the parent resource
 * @param[in] child The handle of the child resource to be added to the parent resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_ALREADY  Already done
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_register_resource()
 * @see iotcon_unregister_resource()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_bind_request_handler()
 * @see iotcon_resource_unbind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_bind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child);

/**
 * @brief Unbinds a child resource from the parent resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] parent The handle of the parent resource
 * @param[in] child The handle of the child resource to be unbound from the parent resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_register_resource()
 * @see iotcon_unregister_resource()
 * @see iotcon_resource_bind_interface()
 * @see iotcon_resource_bind_type()
 * @see iotcon_resource_bind_request_handler()
 * @see iotcon_resource_bind_child_resource()
 * @see iotcon_request_handler_cb()
 */
int iotcon_resource_unbind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child);

/**
 * @brief Register device information in a server.
 *
 * @since_tizen 3.0
 *
 * @param[in] device_name The device information to register
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 */
int iotcon_register_device_info(const char *device_name);

/**
 * @brief Specifies the type of function passed to iotcon_get_device_info().
 *
 * @since_tizen 3.0
 *
 * @param[in] info The information of device from remote server.
 * @param[in] user_data The user data to pass to the function
 *
 * @pre iotcon_get_device_info() will invoke this callback function.
 *
 * @see iotcon_get_device_info()
 */
typedef void (*iotcon_device_info_cb)(const char *device_name, const char *sid,
		const char *spec_version, const char *data_model_version, void *user_data);

/**
 * @brief Calls a function for device information of remote server.
 * @details Request device information to server and pass the information by calling
 * iotcon_device_info_cb().\n
 * iotcon_device_info_cb() will be called when success on getting device information.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] host_address The host address of remote server
 * @param[in] cb The callback function to invoke
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post iotcon_device_info_cb() will be called when success on getting device information.
 *
 * @see iotcon_device_info_cb()
 */
int iotcon_get_device_info(const char *host_address, iotcon_device_info_cb cb,
		void *user_data);

/**
 * @brief Register platform information in a server.
 *
 * @since_tizen 3.0
 *
 * @param[in] platform_info The platform information to register
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 */
int iotcon_register_platform_info(iotcon_platform_info_s *platform_info);

/**
 * @brief Specifies the type of function passed to iotcon_get_platform_info().
 *
 * @since_tizen 3.0
 *
 * @param[in] info The information of platform from remote server.
 * @param[in] user_data The user data to pass to the function
 *
 * @pre iotcon_get_platform_info() will invoke this callback function.
 *
 * @see iotcon_get_platform_info()
 */
typedef void (*iotcon_platform_info_cb)(iotcon_platform_info_s *info, void *user_data);

/**
 * @brief Calls a function for platform information of remote server.
 * @details Request platform information to server and pass the information by calling
 * iotcon_platform_info_cb().\n
 * iotcon_platform_info_cb() will be called when success on getting device information.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] host_address The host address of remote server
 * @param[in] cb The callback function to invoke
 * @param[in] user_data The user data to pass to the function
 *
 * @return  0 on success, otherwise a negative error value.
 *
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post iotcon_platform_info_cb() will be called when success on getting device information.
 *
 * @see iotcon_platform_info_cb()
 */
int iotcon_get_platform_info(const char *host_address, iotcon_platform_info_cb cb,
		void *user_data);

/**
 * @brief Starts presence of a server.
 * @details Use this function to send server's announcements to clients.\n
 * Server can call this function when online for the first time or come back from offline to online.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks If @a time_to_live is 0, server will set default value as 60 seconds.\n
 * If @a time_to_live is very big, server will set maximum value as (60 * 60 * 24) seconds.
 * (24 hours)
 *
 * @param[in] time_to_live The interval of announcing presence in seconds.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_stop_presence()
 * @see iotcon_subscribe_presence()
 * @see iotcon_unsubscribe_presence()
 */
int iotcon_start_presence(unsigned int time_to_live);

/**
 * @brief Stop presence of a server.
 * @details Use this function to stop sending server's announcements to clients.
 * Server can call this function when terminating, entering to offline or out of network.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_start_presence()
 * @see iotcon_subscribe_presence()
 * @see iotcon_unsubscribe_presence()
 */
int iotcon_stop_presence(void);

/**
 * @brief Specifies the type of function passed to iotcon_subscribe_presence().
 * @details Called when client receive presence events from the server.
 *
 * @since_tizen 3.0
 *
 * @param[in] result The result code of server's presence
 * @param[in] nonce Current nonce of server's presence
 * @param[in] host_address The address or addressable name of server
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_subscribe_presence()
 *
 * @see iotcon_subscribe_presence()
 */
typedef void (*iotcon_presence_cb)(int result, unsigned int nonce,
		const char *host_address, void *user_data);

/**
 * @brief Subscribes to a server to receive presence events.
 * @details Request to receive presence to an interested server's resource with @a resource_type.\n
 * If succeed to subscribe, iotcon_presence_cb() will be invoked when the server sends presence\n
 * A server sends presence events when adds/removes/alters a resource or start/stop presence.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] host_address The address or addressable name of the server
 * @param[in] resource_type A resource type that a client has interested in
 * @param[in] cb The callback function to invoke
 * @param[in] user_data The user data to pass to the function
 * @param[out] presence_handle The generated presence handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the resource receive presence, iotcon_presence_cb() will be called.
 *
 * @see iotcon_start_presence()
 * @see iotcon_stop_presence()
 * @see iotcon_unsubscribe_presence()
 * @see iotcon_presence_cb()
 */
int iotcon_subscribe_presence(const char *host_address, const char *resource_type,
		iotcon_presence_cb cb, void *user_data, iotcon_presence_h *presence_handle);

/**
 * @brief Unsubscribes to a server's presence events.
 * @details Request not to receive server's presence any more.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] presence_handle The presence handle to be unsubscribed
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_start_presence()
 * @see iotcon_stop_presence()
 * @see iotcon_subscribe_presence()
 * @see iotcon_presence_cb()
 */
int iotcon_unsubscribe_presence(iotcon_presence_h presence_handle);

/**
 * @brief Specifies the type of function passed to iotcon_find_resource().
 * @details Called when a resource is found from the remote server.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of resource which is found
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_find_resource()
 *
 * @see iotcon_find_resource()
 */
typedef void (*iotcon_found_resource_cb)(iotcon_client_h resource, void *user_data);

/**
 * @brief Finds resources.
 * @details Request to find a resource of @a host_address server with @a resource_type.\n
 * If succeed to find the resource, iotcon_found_resource_cb() will be invoked with
 * information of the resource.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] host_address The address or addressable name of server
 * @param[in] resource_type The resource type specified as a filter for the resource
 * @param[in] cb The callback function to invoke
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the resource is found, iotcon_found_resource_cb() will be called.
 *
 * @see iotcon_found_resource_cb()
 */
int iotcon_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb cb, void *user_data);

/**
 * @brief Creates a new resource handle.
 * @details Creates a resource proxy object so that get/put/observe functionality can be used
 * without discovering the object in advance.\n
 * To use this API, you should provide all of the details required to correctly contact and
 * observe the object.\n
 * If not, you should discover the resource object manually.
 *
 * @since_tizen 3.0
 *
 * @param[in] host The host address of the resource
 * @param[in] uri_path The URI path of the resource.
 * @param[in] is_observable Allow observation
 * @param[in] resource_types The resource type of the resource. For example, "core.light"
 * @param[in] resource_interfaces The resource interfaces (whether it is collection etc)
 * @param[out] client_handle Generated resource handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_client_destroy()
 * @see iotcon_client_ref()
 */
int iotcon_client_create(const char *host,
		const char *uri_path,
		bool is_observable,
		iotcon_resource_types_h resource_types,
		int resource_ifs,
		iotcon_client_h *client_handle);

/**
 * @brief Releases a resource handle.
 * @details Decrements reference count of the source resource.\n
 * If the reference count drops to 0, releases a resource handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 *
 * @return void
 *
 * @see iotcon_client_create()
 * @see iotcon_client_ref()
 */
void iotcon_client_destroy(iotcon_client_h resource);

/**
 * @brief Increments reference count of the source resource.
 *
 * @since_tizen 3.0
 *
 * @param[in] src The Source of resource
 * @param[out] dest The referenced resource handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_client_create()
 * @see iotcon_client_destroy()
 */
int iotcon_client_ref(iotcon_client_h src, iotcon_client_h *dest);

/**
 * @brief Specifies the type of function passed to iotcon_observer_start().
 * @details Called when a client receive notifications from a server. The @a response_result could be one of #iotcon_response_result_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] options The handle of the header options
 * @param[in] response_result The response result code
 * @param[in] sequence_number The sequence of notifications from server.
 * @param[in] user_data The user data passed from the callback registration function
 *
 * @pre The callback must be registered using iotcon_observer_start()
 *
 * @see iotcon_observer_start()
 */
typedef void (*iotcon_on_observe_cb)(iotcon_client_h resource,
		iotcon_representation_h repr,
		iotcon_options_h options,
		int response_result,
		int sequence_number,
		void *user_data);

/**
 * @brief Sets observation on the resource
 * @details When server sends notification message, iotcon_on_observe_cb() will be called.
 * The @a observe_type could be one of #iotcon_observe_type_e.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] observe_type The type to specify how client wants to observe.
 * @param[in] query The query to send to server
 * @param[in] cb The callback function to get notifications from server
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the @a resource receive notification message, iotcon_on_observe_cb() will be called.
 *
 * @see iotcon_on_observe_cb()
 * @see iotcon_observer_stop()
 * @see iotcon_notimsg_create()
 * @see iotcon_notify_list_of_observers()
 * @see iotcon_resource_notify_all()
 */
int iotcon_observer_start(iotcon_client_h resource, int observe_type,
		iotcon_query_h query, iotcon_on_observe_cb cb, void *user_data);

/**
 * @brief Cancels the observation on the resource
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_on_observe_cb()
 * @see iotcon_observer_start()
 * @see iotcon_notimsg_create()
 * @see iotcon_notify_list_of_observers()
 * @see iotcon_resource_notify_all()
 */
int iotcon_observer_stop(iotcon_client_h resource);

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
 * @brief Creates a new notifications message handle.
 * @details @a iface could be one of #iotcon_interface_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The handle of the representation
 * @param[in] iface The resource interface
 * @param[out] notimsg_handle The generated notifications message handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_on_observe_cb()
 * @see iotcon_observer_start()
 * @see iotcon_observer_stop()
 * @see iotcon_notify_list_of_observers()
 * @see iotcon_resource_notify_all()
 */
int iotcon_notimsg_create(iotcon_representation_h repr, int iface,
		iotcon_notimsg_h *notimsg_handle);

/**
 * @brief Releases a notifications message handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] msg The handle of the notifications message
 *
 * @return void
 *
 * @see iotcon_notimsg_create()
 */
void iotcon_notimsg_destroy(iotcon_notimsg_h msg);

/**
 * @brief Notifies only specific clients that resource's attributes have changed.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] msg The handle of the notifications message
 * @param[in] observers The handle of the observers
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_REPRESENTATION  Representation error
 * @retval #IOTCON_ERROR_SYSTEM  System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_on_observe_cb()
 * @see iotcon_observer_start()
 * @see iotcon_observer_stop()
 * @see iotcon_notimsg_create()
 * @see iotcon_resource_notify_all()
 */
int iotcon_notify_list_of_observers(iotcon_resource_h resource, iotcon_notimsg_h msg,
		iotcon_observers_h observers);

/**
 * @brief Notifies all that attributes of the resource have changed.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus error
 * @retval #IOTCON_ERROR_SYSTEM  System error
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @see iotcon_on_observe_cb()
 * @see iotcon_observer_start()
 * @see iotcon_observer_stop()
 * @see iotcon_notimsg_create()
 * @see iotcon_notify_list_of_observers()
 */
int iotcon_resource_notify_all(iotcon_resource_h resource);

/**
 * @brief Specifies the type of function passed to iotcon_get(), iotcon_put(), iotcon_post()
 * @details The @a response_result could be one of #iotcon_response_result_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] options The handle of the header options
 * @param[in] response_result The response result code
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_get(), iotcon_put(), iotcon_post()
 *
 * @see iotcon_get()
 * @see iotcon_put()
 * @see iotcon_post()
 */
typedef void (*iotcon_on_cru_cb)(iotcon_client_h resource, iotcon_representation_h repr,
		iotcon_options_h options, int response_result, void *user_data);

/**
 * @brief Gets the attributes of a resource.
 * @details When server sends response on get request, iotcon_on_cru_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] query The query to send to server
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive get response, iotcon_on_cru_cb() will be called.
 *
 * @see iotcon_on_cru_cb()
 * @see iotcon_put()
 * @see iotcon_post()
 */
int iotcon_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data);

/**
 * @brief Sets the representation of a resource (via PUT)
 * @details When server sends response on put request, iotcon_on_cru_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] query The query to send to server
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive put response, iotcon_on_cru_cb() will be called.
 *
 * @see iotcon_on_cru_cb()
 * @see iotcon_get()
 * @see iotcon_post()
 */
int iotcon_put(iotcon_client_h resource, iotcon_representation_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data);

/**
 * @brief Posts on a resource
 * @details When server sends response on post request, iotcon_on_cru_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] repr The handle of the representation
 * @param[in] query The query to send to server
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive post response, iotcon_on_cru_cb() will be called.
 *
 * @see iotcon_on_cru_cb()
 * @see iotcon_get()
 * @see iotcon_put()
 */
int iotcon_post(iotcon_client_h resource, iotcon_representation_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data);

/**
 * @brief Specifies the type of function passed to iotcon_delete()
 * @details The @a response_result could be one of #iotcon_response_result_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] resource The handle of the resource
 * @param[in] options The handle of the header options
 * @param[in] response_result The response result code
 * @param[in] user_data The user data to pass to the function
 *
 * @pre The callback must be registered using iotcon_delete()
 *
 * @see iotcon_delete()
 */
typedef void (*iotcon_on_delete_cb)(iotcon_client_h resource, iotcon_options_h options,
		int response_result, void *user_data);

/**
 * @brief Deletes a resource.
 * @details When server sends response on delete request, iotcon_on_delete_cb() will be called.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/internet
 *
 * @param[in] resource The handle of the resource
 * @param[in] cb The callback function
 * @param[in] user_data The user data to pass to the function
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_DBUS  Dbus errors
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @post When the client receive delete response, iotcon_on_delete_cb() will be called.
 *
 * @see iotcon_on_delete_cb()
 */
int iotcon_delete(iotcon_client_h resource, iotcon_on_delete_cb cb, void *user_data);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_MANAGER_H__ */
