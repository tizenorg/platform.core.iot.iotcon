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

#include <tizen.h>

#ifndef TIZEN_ERROR_IOTCON
#define TIZEN_ERROR_IOTCON -0x09000000
#endif

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @brief Enumeration for Iotcon error code.
 * @since_tizen 3.0
 */
typedef enum
{
	IOTCON_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
	IOTCON_ERROR_IO_ERROR = TIZEN_ERROR_IO_ERROR, /**< I/O error */
	IOTCON_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	IOTCON_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED, /**< Permission denied */
	IOTCON_ERROR_NOT_SUPPORTED = TIZEN_ERROR_NOT_SUPPORTED, /**< Not supported */
	IOTCON_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	IOTCON_ERROR_NO_DATA = TIZEN_ERROR_NO_DATA, /**< No data available */
	IOTCON_ERROR_TIMEOUT = TIZEN_ERROR_TIMED_OUT, /**< Time out */
	IOTCON_ERROR_IOTIVITY = TIZEN_ERROR_IOTCON | 0x01, /**< Iotivity errors */
	IOTCON_ERROR_REPRESENTATION = TIZEN_ERROR_IOTCON | 0x02, /**< Representation errors */
	IOTCON_ERROR_INVALID_TYPE = TIZEN_ERROR_IOTCON | 0x03, /**< Invalid type */
	IOTCON_ERROR_ALREADY = TIZEN_ERROR_IOTCON | 0x04, /**< Already */
	IOTCON_ERROR_DBUS = TIZEN_ERROR_IOTCON | 0x05, /**< D-Bus errors */
	IOTCON_ERROR_SYSTEM = TIZEN_ERROR_IOTCON | 0x06, /**< System errors */
}iotcon_error_e;

#endif /* __IOT_CONNECTIVITY_MANAGER_ERRORS_H__ */
