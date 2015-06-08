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

typedef enum
{
	IOTCON_ERR_NONE = 0,
	IOTCON_ERR_MEMORY = -1,
	IOTCON_ERR_GDBUS = -2,
	IOTCON_ERR_ENVIRONMENT = -3,
	IOTCON_ERR_SYSTEM = -4,
	IOTCON_ERR_PLUGIN = -5,
	IOTCON_ERR_PARAM = -6,
	IOTCON_ERR_ALREADY = -7,
	IOTCON_ERR_BUSY = -8,
	IOTCON_ERR_NOT_SUPPORT = -9,
	IOTCON_ERR_DISABLE = -10,
	IOTCON_ERR_IOTIVITY = -11,
	IOTCON_ERR_NO_DATA = -12
}iotcon_error_e;

#endif //__IOT_CONNECTIVITY_MANAGER_ERRORS_H__
