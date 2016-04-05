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
#include "iotcon-types.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-request.h"

API int iotcon_request_get_host_address(iotcon_request_h request,
		char **host_address)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER, "host_address is NULL");

	*host_address = request->host_address;

	return IOTCON_ERROR_NONE;
}


API int iotcon_request_get_connectivity_type(iotcon_request_h request,
		iotcon_connectivity_type_e *connectivity_type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == connectivity_type, IOTCON_ERROR_INVALID_PARAMETER,
			"connectivity_type is NULL");

	*connectivity_type = request->connectivity_type;

	return IOTCON_ERROR_NONE;
}


/* The content of the request should not be freed by user. */
API int iotcon_request_get_representation(iotcon_request_h request,
		iotcon_representation_h *repr)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER, "repr is NULL");

	*repr = request->repr;

	return IOTCON_ERROR_NONE;
}


API int iotcon_request_get_request_type(iotcon_request_h request,
		iotcon_request_type_e *type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == type, IOTCON_ERROR_INVALID_PARAMETER, "type is NULL");

	*type = request->type;

	return IOTCON_ERROR_NONE;
}


/* The content of the request should not be freed by user. */
API int iotcon_request_get_options(iotcon_request_h request,
		iotcon_options_h *options)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER, "options is NULL");

	*options = request->header_options;

	return IOTCON_ERROR_NONE;
}


/* The content of the request should not be freed by user. */
API int iotcon_request_get_query(iotcon_request_h request, iotcon_query_h *query)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER, "query is NULL");

	*query = request->query;

	return IOTCON_ERROR_NONE;
}


API int iotcon_request_get_observe_type(iotcon_request_h request,
		iotcon_observe_type_e *observe_type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == observe_type, IOTCON_ERROR_INVALID_PARAMETER, "observe_type is NULL");

	*observe_type = request->observation_info.action;

	return IOTCON_ERROR_NONE;
}


API int iotcon_request_get_observe_id(iotcon_request_h request, int *observe_id)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETVM_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER, "request is NULL");
	RETVM_IF(NULL == observe_id, IOTCON_ERROR_INVALID_PARAMETER, "observe_id is NULL");

	*observe_id = request->observation_info.observe_id;

	return IOTCON_ERROR_NONE;
}

