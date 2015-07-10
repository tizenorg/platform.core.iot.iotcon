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
#include <json-glib/json-glib.h>

#include "iotcon-struct.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-ioty.h"
#include "icl-options.h"
#include "icl-resource-types.h"
#include "icl-dbus.h"
#include "icl-repr.h"
#include "icl-client.h"

/* The length of resource_type should be less than or equal to 61.
 * If resource_type is NULL, then All resources in host are discovered. */
API int iotcon_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	if (resource_type && (IOTCON_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type))) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = icl_dbus_find_resource(host_address, resource_type, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_find_resource() Fail(%d)", ret);

	return ret;
}


/* If you know the information of resource, then you can make a proxy of the resource. */
API iotcon_client_h iotcon_client_new(const char *host, const char *uri_path,
		bool is_observable, iotcon_resource_types_h resource_types, int resource_ifs)
{
	FN_CALL;
	iotcon_client_h resource = NULL;

	RETV_IF(NULL == host, NULL);
	RETV_IF(NULL == uri_path, NULL);
	RETV_IF(NULL == resource_types, NULL);

	resource = calloc(1, sizeof(struct icl_remote_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	resource->host = ic_utils_strdup(host);
	resource->uri_path = ic_utils_strdup(uri_path);
	resource->is_observable = is_observable;
	resource->types = icl_resource_types_ref(resource_types);
	resource->ifaces = resource_ifs;

	return resource;
}


API void iotcon_client_free(iotcon_client_h resource)
{
	FN_CALL;

	RET_IF(NULL == resource);

	free(resource->uri_path);
	free(resource->host);
	free(resource->sid);

	/* null COULD be allowed */
	if (resource->header_options)
		iotcon_options_free(resource->header_options);
	iotcon_resource_types_free(resource->types);
	free(resource);
}


API iotcon_client_h iotcon_client_clone(iotcon_client_h resource)
{
	iotcon_client_h clone;

	RETV_IF(NULL == resource, NULL);

	clone = iotcon_client_new(resource->host,
			resource->uri_path,
			resource->is_observable,
			iotcon_resource_types_clone(resource->types),
			resource->ifaces);
	if (NULL == clone) {
		ERR("iotcon_client_new() Fail");
		return clone;
	}

	clone->sid = resource->sid;
	clone->observe_handle = resource->observe_handle;

	return clone;
}


/* The content of the resource should not be freed by user. */
API int iotcon_client_get_uri_path(iotcon_client_h resource, char **uri_path)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = resource->uri_path;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_client_get_host(iotcon_client_h resource, char **host)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == host, IOTCON_ERROR_INVALID_PARAMETER);

	*host = resource->host;

	return IOTCON_ERROR_NONE;
}

API int iotcon_client_get_server_id(iotcon_client_h resource, char **sid)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == sid, IOTCON_ERROR_INVALID_PARAMETER);

	*sid = resource->sid;

	return IOTCON_ERROR_NONE;
}

/* The content of the resource should not be freed by user. */
API int iotcon_client_get_types(iotcon_client_h resource, iotcon_resource_types_h *types)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = resource->types;

	return IOTCON_ERROR_NONE;
}


API int iotcon_client_get_interfaces(iotcon_client_h resource, int *ifaces)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = resource->ifaces;

	return IOTCON_ERROR_NONE;
}


API int iotcon_client_is_observable(iotcon_client_h resource, bool *observable)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == observable, IOTCON_ERROR_INVALID_PARAMETER);

	*observable = resource->is_observable;

	return IOTCON_ERROR_NONE;
}


/* if header_options is NULL, then client's header_options is unset */
API int iotcon_client_set_options(iotcon_client_h resource,
		iotcon_options_h header_options)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (resource->header_options)
		iotcon_options_free(resource->header_options);

	if (header_options)
		resource->header_options = icl_options_ref(header_options);
	else
		resource->header_options = NULL;

	return IOTCON_ERROR_NONE;
}


API int iotcon_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_dbus_get(resource, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_get() Fail(%d)", ret);

	return ret;
}


API int iotcon_put(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_dbus_put(resource, repr, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_put() Fail(%d)", ret);

	return ret;
}


API int iotcon_post(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_dbus_post(resource, repr, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_post() Fail(%d)", ret);

	return ret;
}


API int iotcon_delete(iotcon_client_h resource, iotcon_on_delete_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_dbus_delete(resource, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_delete() Fail(%d)", ret);

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

	ret = icl_dbus_observer_start(resource, observe_type, query, cb, user_data);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_observer_start() Fail(%d)", ret);

	return ret;
}


API int iotcon_observer_stop(iotcon_client_h resource)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (NULL == resource->observe_handle) {
		ERR("It doesn't have a observe_handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = icl_dbus_observer_stop(resource->observe_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_dbus_observer_stop() Fail(%d)", ret);
		return ret;
	}
	resource->observe_handle = NULL;

	return ret;
}


iotcon_client_h icl_client_parse_resource_object(JsonParser *parser, char *json_string,
		const char *host, iotcon_connectivity_type_e conn_type)
{
	FN_CALL;
	int ret, observable;
	GError *error = NULL;
	iotcon_client_h client;
	const char *uri_path, *server_id;
	int ifaces = IOTCON_INTERFACE_NONE;
	JsonObject *rsrc_obj, *property_obj;
	iotcon_resource_types_h res_types = NULL;

	DBG("input str : %s", json_string);

	ret = json_parser_load_from_data(parser, json_string, strlen(json_string), &error);
	if (FALSE == ret) {
		ERR("json_parser_load_from_data() Fail(%s)", error->message);
		g_error_free(error);
		return NULL;
	}

	rsrc_obj = json_node_get_object(json_parser_get_root(parser));

	uri_path = json_object_get_string_member(rsrc_obj, IC_JSON_KEY_URI_PATH);
	server_id = json_object_get_string_member(rsrc_obj, IC_JSON_KEY_SERVERID);
	if (NULL == server_id) {
		ERR("Invalid Server ID");
		return NULL;
	}

	/* parse resources type and interfaces */
	property_obj = json_object_get_object_member(rsrc_obj, IC_JSON_KEY_PROPERTY);
	if (property_obj) {
		ret = icl_repr_parse_resource_property(property_obj, &res_types, &ifaces);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_repr_parse_resource_property() Fail(%d)", ret);
			return NULL;
		}
	}

	/* parse observable */
	observable = json_object_get_int_member(rsrc_obj, IC_JSON_KEY_OBSERVABLE);

	client = iotcon_client_new(host, uri_path, !!observable, res_types, ifaces);
	if (res_types)
		iotcon_resource_types_free(res_types);

	if (NULL == client) {
		ERR("iotcon_client_new() Fail");
		return NULL;
	}

	client->sid = strdup(server_id);
	if (NULL == client->sid) {
		ERR("strdup(sid) Fail(%d)", errno);
		iotcon_client_free(client);
		return NULL;
	}
	client->conn_type = conn_type;

	return client;
}
