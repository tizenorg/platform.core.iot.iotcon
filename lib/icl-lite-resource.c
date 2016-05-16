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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iotcon-types.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-representation.h"
#include "icl-attributes.h"
#include "icl-value.h"
#include "icl-list.h"
#include "icl-resource.h"
#include "icl-response.h"
#include "icl-lite-resource.h"
#include "icl-ioty.h"


/* The length of uri_path should be less than or equal to 36. */
API int iotcon_lite_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		uint8_t policies,
		iotcon_attributes_h attributes,
		iotcon_lite_resource_post_request_cb cb,
		void *user_data,
		iotcon_lite_resource_h *resource_handle)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(false == ic_utils_check_permission(IC_PERMISSION_INTERNET),
			IOTCON_ERROR_PERMISSION_DENIED);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(false == icl_resource_check_uri_path(uri_path),
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == res_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_handle, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_ioty_lite_resource_create(uri_path, res_types, policies, attributes, cb,
			user_data, resource_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_lite_resource_create() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_destroy(iotcon_lite_resource_h resource)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(false == ic_utils_check_permission(IC_PERMISSION_INTERNET),
			IOTCON_ERROR_PERMISSION_DENIED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_ioty_lite_resource_destroy(resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_lite_resource_destroy() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_update_attributes(iotcon_lite_resource_h resource,
		iotcon_attributes_h attributes)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(false == ic_utils_check_permission(IC_PERMISSION_INTERNET),
			IOTCON_ERROR_PERMISSION_DENIED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_ioty_lite_resource_update_attributes(resource, attributes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_lite_resource_update_attributes() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_lite_resource_get_attributes(iotcon_lite_resource_h resource,
		iotcon_attributes_h *attributes)
{
	RETV_IF(false == ic_utils_check_oic_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == attributes, IOTCON_ERROR_INVALID_PARAMETER);

	*attributes = resource->attributes;

	return IOTCON_ERROR_NONE;
}
