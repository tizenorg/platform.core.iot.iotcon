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
 * @addtogroup CAPI_IOT_CONNECTIVITY_MODULE
 *
 * @{
 */

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
	IOTCON_CONNECTIVITY_EDR, /**< Indicates Bluetooth Enhanced Data Rate connectivity */
	IOTCON_CONNECTIVITY_LE, /**< Indicates Bluetooth Low Energy connectivity */
	IOTCON_CONNECTIVITY_ALL, /**< Indicates all (IPV4 + IPV6 + EDR + LE) connectivities */
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
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_CONSTANT_H__ */
