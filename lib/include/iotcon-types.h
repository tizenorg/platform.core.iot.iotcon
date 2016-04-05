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
#ifndef __IOT_CONNECTIVITY_MANAGER_TYPES_H__
#define __IOT_CONNECTIVITY_MANAGER_TYPES_H__

#include <tizen_type.h>

#include <iotcon-constant.h>

/**
 * @file iotcon-types.h
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
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_COMMON_REPRESENTATION_STATE_LIST_MODULE,
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
 * handle. If observable attribute of resource is true, client can observe this resource.
 *
 * @since_tizen 3.0
 */
typedef struct icl_lite_resource* iotcon_lite_resource_h;

/**
* @brief The handle of response.
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
* @brief The handle of request.
* @details @a iotcon_request_h is an opaque data structure to request to a particular resource.
* @a iotcon_request_h is a data type of client's request which consists of header options,
* query, representation.
*
* @since_tizen 3.0
*/
typedef struct icl_resource_request* iotcon_request_h;

/**
 * @brief The handle of remote resource.
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
 * @brief The handle of presence response.
 * @details @a iotcon_presence_response_h is a handle of presence response subscription.\n
 * It is used to get the information of presence response from server.
 *
 * @since_tizen 3.0
 */
typedef struct icl_presence_response* iotcon_presence_response_h;

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
 * @brief The handle of resource interface.
 * @details @a iotcon_resource_interfaces_h is an opaque data structure to have list
 * of resource interfaces. A resource interface is datatype of string.
 *
 * @since_tizen 3.0
 */
typedef struct icl_resource_ifaces* iotcon_resource_interfaces_h;

/**
 * @brief The handle of resource types.
 * @details @a iotcon_resource_types_h is an opaque data structure to have list
 * of resource types. A resource type is datatype of string.
 *
 * @since_tizen 3.0
 */
typedef struct icl_resource_types* iotcon_resource_types_h;

/**
 * @brief The handle of options.
 * @details @a iotcon_options_h is an opaque data structure to have attribute value map
 * which consists of a key and a value.
 * Datatype of key is integer and value is string.
 *
 * @since_tizen 3.0
 */
typedef struct icl_options* iotcon_options_h;

/**
 * @brief The handle of query.
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
 * @}
 */

#include <iotcon-list.h>
#include <iotcon-query.h>
#include <iotcon-state.h>
#include <iotcon-options.h>
#include <iotcon-representation.h>
#include <iotcon-resource-types.h>
#include <iotcon-resource-interfaces.h>

#endif /* __IOT_CONNECTIVITY_MANAGER_TYPES_H__ */
