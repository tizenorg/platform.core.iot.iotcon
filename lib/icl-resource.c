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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <glib.h>
#include <tizen_type.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-representation.h"
#include "icl-request.h"
#include "icl-resource-types.h"
#include "icl-resource-interfaces.h"
#include "icl-resource.h"
#include "icl-ioty.h"

bool icl_resource_check_uri_path(const char *uri_path)
{
	int i;

	RETV_IF(NULL == uri_path, false);

	if (ICL_URI_PATH_LENGTH_MAX < strlen(uri_path)) {
		ERR("The length of URI path(%s) should be less than or equal to %d.", uri_path,
				ICL_URI_PATH_LENGTH_MAX);
		return false;
	}

	if ('/' != uri_path[0]) {
		ERR("URI path should start with '/'");
		return false;
	}

	for (i = 0; '\0' != uri_path[i]; i++) {
		if ('?' == uri_path[i]) {
			ERR("'?' is not allowed");
			return false;
		}
	}

	return true;
}


static bool _check_type_interface(const char *src)
{
	int i;

	if (src[0] < 'a' || 'z' < src[0]) {
		ERR("'%c' is not allowed", src[0]);
		return false;
	}

	for (i = 1; '\0' != src[i]; i++) {
		if ('.' == src[i])
			continue;
		if ('-' == src[i])
			continue;
		if ((src[i] < 'a' || 'z' < src[i]) && (src[i] < '0' || '9' < src[i])) {
			ERR("'%c' is not allowed", src[i]);
			return false;
		}
	}
	return true;
}


bool icl_resource_check_type(const char *type)
{
	RETV_IF(NULL == type, false);

	if (ICL_RESOURCE_TYPE_LENGTH_MAX < strlen(type)) {
		ERR("The length of type(%s) should be less than or equal to %d.", type,
				ICL_RESOURCE_TYPE_LENGTH_MAX);
		return false;
	}

	return _check_type_interface(type);
}


bool icl_resource_check_interface(const char *iface)
{
	RETV_IF(NULL == iface, false);

	return _check_type_interface(iface);
}


/* The length of uri_path should be less than or equal to 36. */
API int iotcon_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		iotcon_resource_interfaces_h ifaces,
		int properties,
		iotcon_request_handler_cb cb,
		void *user_data,
		iotcon_resource_h *resource_handle)
{
	int ret;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(false == icl_resource_check_uri_path(uri_path),
			IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == res_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_handle, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_ioty_resource_create(uri_path, res_types, ifaces, properties, cb,
			user_data, resource_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_resource_create() Fail(%d)", ret);
		return ret;
	}
	(*resource_handle)->connectivity_type = IOTCON_CONNECTIVITY_ALL;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_destroy(iotcon_resource_h resource)
{
	FN_CALL;
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_destroy(resource);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_destroy() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_bind_interface(iotcon_resource_h resource,
		const char *iface)
{
	FN_CALL;
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == iface, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_bind_interface(resource, iface);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_bind_interface() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_bind_type(iotcon_resource_h resource,
		const char *resource_type)
{
	FN_CALL;
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_type, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_bind_type(resource, resource_type);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_bind_type() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_resource_set_request_handler(iotcon_resource_h resource,
		iotcon_request_handler_cb cb, void *user_data)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	DBG("Request handler is changed");
	resource->cb = cb;
	resource->user_data = user_data;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_bind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child)
{
	int i, ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(parent == child, IOTCON_ERROR_INVALID_PARAMETER);

	for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
		if (child == parent->children[i]) {
			ERR("Child resource was already bound to parent resource.");
			return IOTCON_ERROR_ALREADY;
		}
	}

	connectivity_type = parent->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_bind_child_resource(parent, child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_bind_child_resource() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_unbind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child)
{
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = parent->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_unbind_child_resource(parent, child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_unbind_child_resource() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_number_of_children(iotcon_resource_h resource,
		int *number)
{
	int i;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == number, IOTCON_ERROR_INVALID_PARAMETER);

	*number = 0;
	for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
		if (resource->children[i])
			*number += 1;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_nth_child(iotcon_resource_h parent, int index,
		iotcon_resource_h *child)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);
	if ((index < 0) || (ICL_CONTAINED_RESOURCES_MAX <= index)) {
		ERR("Invalid index(%d)", index);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*child = parent->children[index];

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_resource_get_uri_path(iotcon_resource_h resource, char **uri_path)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = resource->uri_path;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_resource_get_types(iotcon_resource_h resource,
		iotcon_resource_types_h *types)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = resource->types;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_interfaces(iotcon_resource_h resource,
		iotcon_resource_interfaces_h *ifaces)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = resource->ifaces;

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_get_properties(iotcon_resource_h resource, int *properties)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == properties, IOTCON_ERROR_INVALID_PARAMETER);

	*properties = resource->properties;

	return IOTCON_ERROR_NONE;
}

API int iotcon_resource_notify(iotcon_resource_h resource,
		iotcon_representation_h repr, iotcon_observers_h observers, iotcon_qos_e qos)
{
	int ret, connectivity_type;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = resource->connectivity_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_resource_notify(resource, repr, observers, qos);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_resource_notify() Fail(%d)", ret);
			return ret;
		}
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

