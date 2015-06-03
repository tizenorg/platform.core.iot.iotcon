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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "iotcon-struct.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-ioty.h"
#include "ic-options.h"
#include "ic-client.h"

/* host address should begin with "coap://"
 * The length of resource_type should be less than and equal to 61.
 * If resource_type is NULL, then All resources in host are discovered. */
API int iotcon_find_resource(const char *host_addr, const char *resource_type,
		iotcon_found_resource_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == host_addr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	if (resource_type && (IOTCON_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type))) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = ic_ioty_find_resource(host_addr, resource_type, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_find_resource() Fail(%d)", ret);

	return ret;
}


/* If you know the information of resource, then you can make a proxy of the resource. */
API iotcon_client_h iotcon_client_new(const char *host,
		const char *uri,
		bool is_observable,
		iotcon_str_list_s *resource_types,
		iotcon_interface_e resource_ifs)
{
	FN_CALL;
	int i;
	iotcon_client_h resource = NULL;

	RETV_IF(NULL == host, NULL);
	RETV_IF(NULL == uri, NULL);
	RETV_IF(NULL == resource_types, NULL);

	for (i = 0; i < iotcon_str_list_length(resource_types); i++) {
		if (IOTCON_RESOURCE_TYPE_LENGTH_MAX
				< strlen(iotcon_str_list_nth_data(resource_types, i))) {
			ERR("The length of resource_type is invalid");
			return NULL;
		}
	}

	resource = calloc(1, sizeof(struct ic_remote_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	resource->host = ic_utils_strdup(host);
	resource->uri = ic_utils_strdup(uri);
	resource->is_observable = is_observable;
	resource->types = resource_types;
	resource->ifaces = resource_ifs;

	return resource;
}


API void iotcon_client_free(iotcon_client_h resource)
{
	FN_CALL;

	RET_IF(NULL == resource);

	free(resource->uri);
	free(resource->host);
	ic_options_free(resource->header_options);
	iotcon_str_list_free(resource->types);
	free(resource);
}


API iotcon_client_h iotcon_client_clone(iotcon_client_h resource)
{
	iotcon_client_h clone;

	RETV_IF(NULL == resource, NULL);

	clone = iotcon_client_new(resource->host,
			resource->uri,
			resource->is_observable,
			iotcon_str_list_clone(resource->types),
			resource->ifaces);

	clone->observe_handle = resource->observe_handle;

	return clone;
}


API const char* iotcon_client_get_uri(iotcon_client_h resource)
{
	RETV_IF(NULL == resource, NULL);

	return resource->uri;
}


API const char* iotcon_client_get_host(iotcon_client_h resource)
{
	RETV_IF(NULL == resource, NULL);

	return resource->host;
}


API iotcon_str_list_s* iotcon_client_get_types(iotcon_client_h resource)
{
	RETV_IF(NULL == resource, NULL);

	return resource->types;
}


API int iotcon_client_get_interfaces(iotcon_client_h resource)
{
	RETV_IF(NULL == resource, IOTCON_INTERFACE_NONE);

	return resource->ifaces;
}


/* if header_options is NULL, then client's header_options is unset */
API int iotcon_client_set_options(iotcon_client_h resource,
		iotcon_options_h header_options)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (resource->header_options)
		iotcon_options_free(resource->header_options);

	resource->header_options = header_options;

	return IOTCON_ERROR_NONE;
}


API int iotcon_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_get_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = ic_ioty_get(resource, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_get() Fail(%d)", ret);

	return ret;
}


API int iotcon_put(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_put_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = ic_ioty_put(resource, repr, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_put() Fail(%d)", ret);

	return ret;
}


API int iotcon_post(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_post_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = ic_ioty_post(resource, repr, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_post() Fail(%d)", ret);

	return ret;
}


API int iotcon_delete(iotcon_client_h resource, iotcon_on_delete_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = ic_ioty_delete_res(resource, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_delete_res() Fail(%d)", ret);

	return ret;
}


API int iotcon_observer_start(iotcon_client_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_on_observe_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = ic_ioty_observe(resource, observe_type, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_observe() Fail(%d)", ret);

	return ret;
}


API int iotcon_observer_stop(iotcon_client_h resource)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = ic_ioty_cancel_observe(resource);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_cancel_observe() Fail(%d)", ret);

	return ret;
}
