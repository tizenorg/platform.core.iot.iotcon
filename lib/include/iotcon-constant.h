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
 * @brief HeaderOption range from 2048 t0 3000
 * NOTE: HeaderOptionID  is an unsigned integer value which MUST be within
 * range of 2048 to 3000 inclusive of lower and upper bound.
 * HeaderOptions instance creation fails if above condition is not satisfied.
 */
#define IOTCON_OPTIONID_MIN 2048
#define IOTCON_OPTIONID_MAX 3000


/**
 * @brief Action associated with observation
 */
typedef enum {
	IOTCON_OBSERVE_REGISTER = 0,
	IOTCON_OBSERVE_DEREGISTER = 1,
	IOTCON_OBSERVE_NO_OPTION = 2
} iotcon_osbserve_action_e;

typedef enum {
	IOTCON_OBSERVE = 0,
	IOTCON_OBSERVE_ALL = 1
} iotcon_observe_type_e;

typedef enum {
	IOTCON_INTERFACE_DEFAULT = 0,
	IOTCON_INTERFACE_LINK = 1,
	IOTCON_INTERFACE_BATCH = 2,
	IOTCON_INTERFACE_GROUP = 3,
	IOTCON_INTERFACE_MAX = 4
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
	IOTCON_RESP_NONE = 0,
	IOTCON_RESP_RES_URI = 1,
	IOTCON_RESP_ERR_CODE = 2,
	IOTCON_RESP_RESULT = 3,
	IOTCON_RESP_REPRESENTATION = 4,
	IOTCON_RESP_HEADER_OPTIONS = 5
} iotcon_response_property_e;

typedef enum {
	IOTCON_INIT_FLAG = (1 << 0),
	IOTCON_REQUEST_FLAG = (1 << 1),
	IOTCON_OBSERVE_FLAG = (1 << 2)
} iotcon_entity_handler_flag_e;

typedef enum {
	IOTCON_EH_OK = 0,
	IOTCON_EH_ERROR,
	IOTCON_EH_RESOURCE_CREATED,
	IOTCON_EH_RESOURCE_DELETED,
	IOTCON_EH_SLOW,
	IOTCON_EH_FORBIDDEN,
	IOTCON_EH_MAX
} iotcon_entity_handler_result_e;

#endif //__IOT_CONNECTIVITY_MANAGER_CONSTANT_H__
