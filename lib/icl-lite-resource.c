/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <glib.h>

#include "iotcon-types.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-representation.h"
#include "icl-state.h"
#include "icl-value.h"
#include "icl-list.h"
#include "icl-resource.h"
#include "icl-response.h"
#include "icl-lite-resource.h"
#include "icl-ioty.h"


/* The length of uri_path should be less than or equal to 36. */
API int iotcon_lite_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		int properties,
		iotcon_state_h state,
		iotcon_lite_resource_post_request_cb cb,
		void *user_data,
		iotcon_lite_resource_h *resource_handle)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(false == icl_resource_check_uri_path(uri_path),
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == res_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_handle, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_ioty_lite_resource_create(uri_path, res_types, properties, state, cb,
			user_data, resource_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_lite_resource_create() Fail(%d)", ret);
		return ret;
	}
	(*resource_handle)->connectivity_type = IOTCON_CONNECTIVITY_ALL;

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_destroy(iotcon_lite_resource_h resource)
{
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_lite_resource_destroy(resource);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_lite_resource_destroy() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_update_state(iotcon_lite_resource_h resource,
		iotcon_state_h state)
{
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_lite_resource_update_state(resource, state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_lite_resource_update_state() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_get_state(iotcon_lite_resource_h resource,
		iotcon_state_h *state)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	*state = resource->state;

	return IOTCON_ERROR_NONE;
}
