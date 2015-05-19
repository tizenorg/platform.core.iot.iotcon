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
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-common.h"
#include "ic-struct.h"

API iotcon_repr_h iotcon_request_get_representation(iotcon_request_h request)
{
	RETV_IF(NULL == request, NULL);

	return request->repr;
}


API const char* iotcon_request_get_request_type(iotcon_request_h request)
{
	RETV_IF(NULL == request, NULL);

	return request->request_type;
}


API int iotcon_request_get_request_handler_flag(iotcon_request_h request)
{
	RETV_IF(NULL == request, 0);

	return request->request_handler_flag;
}


API iotcon_options_h iotcon_request_get_options(iotcon_request_h request)
{
	RETV_IF(NULL == request, NULL);

	return request->header_options;
}


API iotcon_query_h iotcon_request_get_query(iotcon_request_h request)
{
	RETV_IF(NULL == request, NULL);

	return request->query;
}


API iotcon_observe_action_e iotcon_request_get_observer_action(iotcon_request_h request)
{
	RETV_IF(NULL == request, IOTCON_OBSERVE_NO_OPTION);

	return request->observation_info.action;
}


API uint8_t iotcon_request_get_observer_id(iotcon_request_h request)
{
	RETV_IF(NULL == request, 0);

	return request->observation_info.observer_id;
}

