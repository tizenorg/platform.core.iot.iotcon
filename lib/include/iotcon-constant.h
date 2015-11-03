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
#ifndef __IOT_CONNECTIVITY_MANAGER_CONSTANT_H__
#define __IOT_CONNECTIVITY_MANAGER_CONSTANT_H__

/**
 * @file iotcon-constant.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_COMMON_MODULE Common
 *
 * @brief Iotcon Common API provides the set of definitions to use server and client API.
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_COMMON_MODULE_OVERVIEW Overview
 * This API set consists of data structures for
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_RESOURCE_TYPES_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_QUERY_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_OPTIONS_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_LIST_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_STATE_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_RESPONSE_MODULE.
 *
 * @{
 */

/**
 * @brief The handle of resource.
 * @details @a iotcon_resource_h is an opaque data structure to represent registered
 * resource by server. A resource has host_address, uri_path, resource types, interfaces
 * and internal handle. If observable attribute of resource is true, client can observe
 * this resource. When client request by CRUD functions, handler will be invoked
 * if registered. It could contain other resource as children.
 *
 * @since_tizen 3.0
 */
typedef struct icl_resource* iotcon_resource_h;

/**
 * @brief The handle of lite resource.
 * @details @a iotcon_lite_resource_h is an opaque data structure to represent registered
 * resource by server. A resource has host_address, uri_path, resource types, and internal
 * handle. If observable attribute of resource is true, client can observe
 * this resource.
 *
 * @since_tizen 3.0
 */
typedef struct icl_lite_resource* iotcon_lite_resource_h;

/**
* @brief The handle of response
* @details @a iotcon_response_h is an opaque data structure to respond to client.
* @a iotcon_response_h is a data type of server's response which consists of result,
* header options, query, representation.
*
*
* @since_tizen 3.0
*/
typedef struct icl_resource_response* iotcon_response_h;

/**
 * @brief The handle of observers.
 * @details The list of observer ids.
 *
 * @since_tizen 3.0
 */
typedef struct icl_observers* iotcon_observers_h;

/**
* @brief The handle of request
* @details @a iotcon_request_h is an opaque data structure to request to a particular resource.
* @a iotcon_request_h is a data type of client's request which consists of header options,
* query, representation.
*
* @since_tizen 3.0
*/
typedef struct icl_resource_request* iotcon_request_h;

/**
 * @brief The handle of remote resource
 * @details When Client success to find out resource from remote server,
 * server's resource information is reorganized as @a iotcon_remote_resource_h by Iotcon.
 * Client can request CRUD to server by using this.
 * @a iotcon_remote_resource_h is an opaque data structure to have host_address, uri_path,
 * resource types, interfaces, options and device id.
 * If observable attribute is true, remote resource is observable.
 * When you observe remote resource, observe_handle will be set.
 *
 * @since_tizen 3.0
 */
typedef struct icl_remote_resource* iotcon_remote_resource_h;

/**
 * @brief The handle of presence.
 * @details @a iotcon_presence_h is a handle of presence subscription.
 * It is used to cancel presence.
 *
 * @since_tizen 3.0
 */
typedef struct icl_presence* iotcon_presence_h;

/**
 * @brief The handle of device information.
 * @details @a iotcon_device_info_h is a handle of device information.
 *
 * @since_tizen 3.0
 */
typedef struct icl_device_info* iotcon_device_info_h;

/**
 * @brief The handle of platform information.
 * @details @a iotcon_platform_info_h is a handle of platform information.
 *
 * @since_tizen 3.0
 */
typedef struct icl_platform_info* iotcon_platform_info_h;

/**
 * @brief The handle of tizen device information.
 * @details @a iotcon_tizen_info_h is a handle of tizen device information.
 *
 * @since_tizen 3.0
 */
typedef struct icl_tizen_info* iotcon_tizen_info_h;

/**
 * @brief The handle of resource types
 * @details @a iotcon_resource_types_h is an opaque data structure to have list
 * of resource types. A resource type is datatype of string.
 *
 * @since_tizen 3.0
 */
typedef struct icl_resource_types* iotcon_resource_types_h;

/**
 * @brief The handle of options
 * @details @a iotcon_options_h is an opaque data structure to have attribute value map
 * which consists of a key and a value.
 * Datatype of key is integer and value is string.
 *
 * @since_tizen 3.0
 */
typedef struct icl_options* iotcon_options_h;

/**
 * @brief The handle of query
 * @details @a iotcon_query_h is an opaque data structure to have attribute value map
 * which consists of key and value.
 * Data ype of both key and value are string.
 * @a iotcon_query_h also have length.
 * The length is total length of all keys and values of map.
 * The length should be less than or equal to 64.
 *
 * @since_tizen 3.0
 */
typedef struct icl_query* iotcon_query_h;

/**
 * @brief The handle of representation.
 * @details @a iotcon_representation_h is an opaque data structure to have uri_path,
 * list of resource types and interfaces.
 * It could contain other representation as children.
 *
 * @since_tizen 3.0
 */
typedef struct icl_representation_s* iotcon_representation_h;

/**
 * @brief The handle of list which is consist of iotcon_value_h type values.
 * @details @a iotcon_list_h is an opaque data structure.
 *
 * @since_tizen 3.0
 */
typedef struct icl_list_s* iotcon_list_h;

/**
 * @brief The handle of state.
 * @details @a iotcon_state_h is an opaque data structure to have attribute value map.
 * Attribute value map consists of a key and a value.
 * Datatype of the key is string and the value should be one of them #IOTCON_TYPE_INT,
 * #IOTCON_TYPE_BOOL, #IOTCON_TYPE_DOUBLE, #IOTCON_TYPE_STR, #IOTCON_TYPE_NULL,
 * #IOTCON_TYPE_LIST and #IOTCON_TYPE_STATE
 *
 * @since_tizen 3.0
 */
typedef struct icl_state_s* iotcon_state_h;

/**
 * @brief The IP Address for multicast.
 *
 * @since_tizen 3.0
 */
#define IOTCON_MULTICAST_ADDRESS "224.0.1.187" /**< Multicast IP Address */

/**
 * @brief Use this value as the return value to stop foreach function.
 *
 * @since_tizen 3.0
 */
#define IOTCON_FUNC_STOP 0

/**
 * @brief Use this value as the return value to continue foreach function.
 *
 * @since_tizen 3.0
 */
#define IOTCON_FUNC_CONTINUE 1

/**
 * @brief Enumeration for action of observation.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_OBSERVE_REGISTER = 0, /**< Indicates action of registering observation*/
	IOTCON_OBSERVE_DEREGISTER = 1, /**< Indicates action of unregistering observation */
	IOTCON_OBSERVE_NO_OPTION = 2 /**< Indicates no option */
} iotcon_observe_action_e;

/**
 * @brief Enumeration for type of observation.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_OBSERVE = 0, /**< Indicates observation request for most up-to-date notifications only */
	IOTCON_OBSERVE_ALL = 1 /**< Indicates observation request for all notifications including stale notifications */
} iotcon_observe_type_e;

/**
 * @brief Enumeration for type of interfaces which can be held in a resource.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_INTERFACE_NONE = 0, /**< Indicates interface not specified or uninitialized */
	IOTCON_INTERFACE_DEFAULT = (1 << 0), /**< Indicates interface for default */
	IOTCON_INTERFACE_LINK = (1 << 1), /**< Indicates interface which is used to retrieve (GET) a list of resources on a server */
	IOTCON_INTERFACE_BATCH = (1 << 2), /**< Indicates interface which is used to to manipulate (GET, PUT, POST, DELETE) a collection of sub-resources at the same time */
	IOTCON_INTERFACE_GROUP = (1 << 3), /**< Indicates interface which is used to to manipulate (GET, PUT, POST) a group of remote resources */
} iotcon_interface_e;

/**
 * @brief Enumeration for of connectivities which can be held in a resource.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_CONNECTIVITY_IPV4 = 0, /**< Indicates Internet Protocol version 4 connectivity */
	IOTCON_CONNECTIVITY_IPV6, /**< Indicates Internet Protocol version 6 connectivity */
	IOTCON_CONNECTIVITY_BT_EDR, /**< Indicates Bluetooth Enhanced Data Rate connectivity */
	IOTCON_CONNECTIVITY_BT_LE, /**< Indicates Bluetooth Low Energy connectivity */
	IOTCON_CONNECTIVITY_ALL, /**< Indicates all connectivities */
} iotcon_connectivity_type_e;

/**
 * @brief Enumeration for property which can be held in a resource.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_HIDDEN = 0, /**< Indicates resource uninitialized */
	IOTCON_DISCOVERABLE = (1 << 0), /**< Indicates resource that is allowed to be discovered */
	IOTCON_OBSERVABLE = (1 << 1), /**< Indicates resource that is allowed to be observed */
	IOTCON_ACTIVE = (1 << 2), /**< Indicates resource initialized and activated */
	IOTCON_SLOW = (1 << 3), /**< Indicates resource which takes some delay to respond */
	IOTCON_SECURE = (1 << 4), /**< Indicates secure resource */
} iotcon_resource_property_e;

/**
 * @brief Enumeration for property which can be held in a response.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_RESPONSE_NEW_URI_PATH = 1, /**< Indicates uri_path which can be held in a response */
	IOTCON_RESPONSE_RESULT = 2, /**< Indicates result which can be held in a response */
	IOTCON_RESPONSE_REPRESENTATION = 3, /**< Indicates representation which can be held in a response */
	IOTCON_RESPONSE_HEADER_OPTIONS = 4, /**< Indicates header options which can be held in a response */
	IOTCON_RESPONSE_INTERFACE = 5, /**< Indicates interface which can be held in a response */
} iotcon_response_property_e;

/**
 * @brief Enumeration for type of request.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_REQUEST_GET = (1 << 0), /**< Indicates get method of request */
	IOTCON_REQUEST_PUT = (1 << 1), /**< Indicates put method of request */
	IOTCON_REQUEST_POST = (1 << 2), /**< Indicates post method of request */
	IOTCON_REQUEST_DELETE = (1 << 3), /**< Indicates delete method of request */
	IOTCON_REQUEST_OBSERVE = (1 << 4), /**< Indicates observe method of request */
} iotcon_request_type_e;

/**
 * @brief Enumeration for result of response.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_RESPONSE_RESULT_OK = 0, /**< Indicates result of response for success */
	IOTCON_RESPONSE_RESULT_ERROR, /**< Indicates result of response for something error */
	IOTCON_RESPONSE_RESULT_RESOURCE_CREATED, /**< Indicates result of response for resource has created */
	IOTCON_RESPONSE_RESULT_RESOURCE_DELETED, /**< Indicates result of response for resource has deleted */
	IOTCON_RESPONSE_RESULT_SLOW, /**< Indicates result of response for slow resource */
	IOTCON_RESPONSE_RESULT_FORBIDDEN, /**< Indicates result of response for accessing unauthorized resource */
} iotcon_response_result_e;

/**
 * @brief Enumeration for result of presence.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_PRESENCE_OK = 0, /**< Indicates for successful action of presence */
	IOTCON_PRESENCE_STOPPED, /**< Indicates for stopped action of presence */
	IOTCON_PRESENCE_TIMEOUT, /**< Indicates for no response of presence for some time */
	IOTCON_PRESENCE_ERROR /**< Indicates for some errors of presence */
} iotcon_presence_result_e;

/**
 * @brief Enumeration for types of representation that is able to have.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_TYPE_NONE = 0, /**< Indicates for representation that have no type */
	IOTCON_TYPE_INT, /**< Indicates for representation that have int type */
	IOTCON_TYPE_BOOL, /**< Indicates for representation that have bool type */
	IOTCON_TYPE_DOUBLE, /**< Indicates for representation that have double type */
	IOTCON_TYPE_STR, /**< Indicates for representation that have string type */
	IOTCON_TYPE_NULL, /**< Indicates for representation that have null type */
	IOTCON_TYPE_LIST, /**< Indicates for representation that have list type */
	IOTCON_TYPE_STATE, /**< Indicates for representation that have another representation type */
} iotcon_types_e;

/**
 * @brief Enumeration for properties of device information.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_DEVICE_INFO_NAME = 0, /**< Indicates human friendly name for device */
	IOTCON_DEVICE_INFO_SPEC_VER, /**< Indicates spec version of the core specification */
	IOTCON_DEVICE_INFO_ID, /**< Indicates unique identifier for OIC device */
	IOTCON_DEVICE_INFO_DATA_MODEL_VER, /**< Indicates version of the specs this device data model is implemented to */
} iotcon_device_info_e;

/**
 * @brief Enumeration for properties of platform information.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_PLATFORM_INFO_ID = 0, /**< Indicates platform identifier */
	IOTCON_PLATFORM_INFO_MANUF_NAME, /**< Indicates name of manufacturer */
	IOTCON_PLATFORM_INFO_MANUF_URL, /**< Indicates URL to manufacturer */
	IOTCON_PLATFORM_INFO_MODEL_NUMBER, /**< Indicates model number as designated by manufacturer */
	IOTCON_PLATFORM_INFO_DATE_OF_MANUF, /**< Indicates manufacturing date of device */
	IOTCON_PLATFORM_INFO_PLATFORM_VER, /**< Indicates version of platform defined by manufacturer */
	IOTCON_PLATFORM_INFO_OS_VER, /**< Indicates version of platform resident OS */
	IOTCON_PLATFORM_INFO_HARDWARE_VER, /**< Indicates version of platform hardware */
	IOTCON_PLATFORM_INFO_FIRMWARE_VER, /**< Indicates version of device firmware */
	IOTCON_PLATFORM_INFO_SUPPORT_URL, /**< Indicates URL that points to support information from manufacturer */
	IOTCON_PLATFORM_INFO_SYSTEM_TIME, /**< Indicates reference time for the device */
} iotcon_platform_info_e;

/**
 * @brief Enumeration for properties of tizen device information.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_TIZEN_INFO_DEVICE_NAME = 0, /**< Indicates human friendly name for device */
	IOTCON_TIZEN_INFO_TIZEN_DEVICE_ID, /**< Indicates randomly generated id value for tizen device */
} iotcon_tizen_info_e;

/**
 * @brief Enumeration for states of remote resource.
 *
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_REMOTE_RESOURCE_STATE_ALIVE, /**< Indicates remote resource is alive */
	IOTCON_REMOTE_RESOURCE_STATE_LOST_SIGNAL, /**< Indicates remote resource is lost */
} iotcon_remote_resource_state_e;

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_CONSTANT_H__ */
