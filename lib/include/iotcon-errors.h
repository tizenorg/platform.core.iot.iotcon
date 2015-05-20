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
#ifndef __IOT_CONNECTIVITY_MANAGER_ERRORS_H__
#define __IOT_CONNECTIVITY_MANAGER_ERRORS_H__

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Enumerations of Iotcon error codes.
 * @since_tizen 3.0
 */
typedef enum
{
	IOTCON_ERROR_NONE = 0, /**< Successful*/
	IOTCON_ERROR_MEMORY = -1, /**< Out of memory */
	IOTCON_ERROR_GDBUS = -2,
	IOTCON_ERROR_ENVIRONMENT = -3,
	IOTCON_ERROR_SYSTEM = -4,
	IOTCON_ERROR_PLUGIN = -5,
	IOTCON_ERROR_PARAM = -6, /**< Invalid parameter */
	IOTCON_ERROR_ALREADY = -7,
	IOTCON_ERROR_BUSY = -8, /**< Device or resource busy */
	IOTCON_ERROR_NOT_SUPPORT = -9,
	IOTCON_ERROR_DISABLE = -10,
	IOTCON_ERROR_IOTIVITY = -11, /**< Iotivity errors */
	IOTCON_ERROR_NO_DATA = -12, /**< No data available */
	IOTCON_ERROR_FAIL = -13, /**< Fail */
}iotcon_error_e;

#endif /* __IOT_CONNECTIVITY_MANAGER_ERRORS_H__ */
