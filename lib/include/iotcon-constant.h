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

#define IOTCON_ALL_INTERFACES "0.0.0.0"
#define IOTCON_RANDOM_PORT 0
#define IOTCON_MULTICAST_ADDRESS "224.0.1.187"

/**
 * @brief HeaderOption range from 2048 to 3000
 * NOTE: HeaderOptionID  is an unsigned integer value which MUST be within
 * range of 2048 to 3000 inclusive of lower and upper bound.
 * HeaderOptions instance creation fails if above condition is not satisfied.
 */
#define IOTCON_OPTIONID_MIN 2048
#define IOTCON_OPTIONID_MAX 3000

#define IOTCON_OPTIONS_MAX 2
#define IOTCON_OPTION_DATA_LENGTH_MAX 16

#define IOTCON_URI_LENGTH_MAX 36

#define IOTCON_QUERY_LENGTH_MAX 64

/* IOTCON_QUERY_LENGTH_MAX - LENGTH("rt=") */
#define IOTCON_RESOURCE_TYPE_LENGTH_MAX (IOTCON_QUERY_LENGTH_MAX - 3)

#define IOTCON_MANUFACTURER_NAME_LENGTH_MAX 15
#define IOTCON_MANUFACTURER_URL_LENGTH_MAX 32

#define IOTCON_CONTAINED_RESOURCES_MAX 5

#define IOTCON_FUNC_STOP 0
#define IOTCON_FUNC_CONTINUE 1

/**
 * @brief Action associated with observation
 */
typedef enum {
	IOTCON_OBSERVE_REGISTER = 0,
	IOTCON_OBSERVE_DEREGISTER = 1,
	IOTCON_OBSERVE_NO_OPTION = 2
} iotcon_observe_action_e;

typedef enum {
	IOTCON_OBSERVE = 0,
	IOTCON_OBSERVE_ALL = 1
} iotcon_observe_type_e;

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Enumerations of Iotcon interface types.
 * @since_tizen 3.0
 */
typedef enum {
	IOTCON_INTERFACE_NONE = 0,
	IOTCON_INTERFACE_DEFAULT = (1 << 0), /* default interface */
	IOTCON_INTERFACE_LINK = (1 << 1), /* discovers children of the parent resource */
	IOTCON_INTERFACE_BATCH = (1 << 2), /* requests CRUD to children of the parent resource */
	IOTCON_INTERFACE_GROUP = (1 << 3), /* requests CRUD to remote resources of a group. */
} iotcon_interface_e;

typedef enum {
	IOTCON_HIDDEN = 0,
	IOTCON_ACTIVE = (1 << 0),
	IOTCON_DISCOVERABLE = (1 << 1),
	IOTCON_OBSERVABLE = (1 << 2),
	IOTCON_SLOW = (1 << 3),
	IOTCON_SECURE = (1 << 4),
} iotcon_resource_property_e;

typedef enum {
	IOTCON_RESPONSE_NONE = 0,
	IOTCON_RESPONSE_RESOURCE_URI = 1,
	IOTCON_RESPONSE_RESULT = 2,
	IOTCON_RESPONSE_REPRESENTATION = 3,
	IOTCON_RESPONSE_HEADER_OPTIONS = 4,
	IOTCON_RESPONSE_INTERFACE = 5,
} iotcon_response_property_e;

typedef enum {
	IOTCON_REQUEST_GET = (1 << 0),
	IOTCON_REQUEST_PUT = (1 << 1),
	IOTCON_REQUEST_POST = (1 << 2),
	IOTCON_REQUEST_DELETE = (1 << 3),
	IOTCON_REQUEST_OBSERVE = (1 << 4),
} iotcon_request_type_e;

typedef enum {
	IOTCON_RESPONSE_RESULT_OK = 0,
	IOTCON_RESPONSE_RESULT_ERROR,
	IOTCON_RESPONSE_RESULT_RESOURCE_CREATED,
	IOTCON_RESPONSE_RESULT_RESOURCE_DELETED,
	IOTCON_RESPONSE_RESULT_SLOW,
	IOTCON_RESPONSE_RESULT_FORBIDDEN,
	IOTCON_RESPONSE_RESULT_MAX
} iotcon_response_result_e;

typedef enum {
	IOTCON_PRESENCE_OK = 0,
	IOTCON_PRESENCE_STOPPED,
	IOTCON_PRESENCE_TIMEOUT,
	IOTCON_PRESENCE_ERROR
} iotcon_presence_result_e;

typedef enum {
	IOTCON_TYPE_NONE = 0,
	IOTCON_TYPE_INT,
	IOTCON_TYPE_BOOL,
	IOTCON_TYPE_DOUBLE,
	IOTCON_TYPE_STR,
	IOTCON_TYPE_NULL,
	IOTCON_TYPE_LIST,
	IOTCON_TYPE_REPR,
} iotcon_types_e;

#endif /* __IOT_CONNECTIVITY_MANAGER_CONSTANT_H__ */
