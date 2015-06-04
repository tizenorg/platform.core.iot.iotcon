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
#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "icl.h"
#include "icl-request.h"

/* The content of the request should not be freed by user. */
API int iotcon_request_get_uri(iotcon_request_h request, char **uri)
{
	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, IOTCON_ERROR_INVALID_PARAMETER);

	*uri = request->uri;

	return IOTCON_ERROR_NONE;
}


/* The content of the request should not be freed by user. */
API int iotcon_request_get_representation(iotcon_request_h request, iotcon_repr_h *repr)
{
	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	*repr = request->repr;

	return IOTCON_ERROR_NONE;
}


API int iotcon_request_get_types(iotcon_request_h request, int *types)
{
	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = request->types;

	return IOTCON_ERROR_NONE;
}


/* The content of the request should not be freed by user. */
API int iotcon_request_get_options(iotcon_request_h request, iotcon_options_h *options)
{
	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER);

	*options = request->header_options;

	return IOTCON_ERROR_NONE;
}


/* The content of the request should not be freed by user. */
API int iotcon_request_get_query(iotcon_request_h request, iotcon_query_h *query)
{
	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);

	*query = request->query;

	return IOTCON_ERROR_NONE;
}


API int iotcon_request_get_observer_action(iotcon_request_h request,
		iotcon_observe_action_e *action)
{
	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == action, IOTCON_ERROR_INVALID_PARAMETER);

	*action = request->observation_info.action;

	return IOTCON_ERROR_NONE;
}


API int iotcon_request_get_observer_id(iotcon_request_h request, int *observer_id)
{
	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == observer_id, IOTCON_ERROR_INVALID_PARAMETER);

	*observer_id = request->observation_info.observer_id;

	return IOTCON_ERROR_NONE;
}

