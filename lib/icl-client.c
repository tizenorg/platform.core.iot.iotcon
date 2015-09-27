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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-options.h"
#include "icl-dbus.h"
#include "icl-repr.h"
#include "icl-client.h"
#include "icl-resource-types.h"
#include "icl-payload.h"

typedef struct {
	iotcon_found_resource_cb cb;
	void *user_data;
	unsigned int id;
} icl_found_resource_s;

static iotcon_client_h _icl_client_from_gvariant(GVariant *payload,
		iotcon_connectivity_type_e conn_type);

static void _icl_found_resource_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	int conn_type;
	iotcon_client_h client;

	GVariant *payload;
	icl_found_resource_s *cb_container = user_data;
	iotcon_found_resource_cb cb = cb_container->cb;

	g_variant_get(parameters, "(vi)", &payload, &conn_type);

	client = _icl_client_from_gvariant(payload, conn_type);
	if (NULL == client) {
		ERR("icl_client_parse_resource_object() Fail");
		return;
	}

	if (cb)
		cb(client, cb_container->user_data);

	iotcon_client_destroy(client);

	/* TODO
	 * When is callback removed?
	 */
}


/* The length of resource_type should be less than or equal to 61.
 * If resource_type is NULL, then All resources in host are discovered. */
API int iotcon_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb cb, void *user_data)
{
	unsigned int sub_id;
	GError *error = NULL;
	int ret, signal_number;
	icl_found_resource_s *cb_container;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	if (resource_type && (ICL_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type))) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	signal_number = icl_dbus_generate_signal_number();

	ic_dbus_call_find_resource_sync(icl_dbus_get_object(), host_address,
			ic_utils_dbus_encode_str(resource_type), signal_number, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_find_resource_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%u", IC_DBUS_SIGNAL_FOUND_RESOURCE,
			signal_number);

	cb_container = calloc(1, sizeof(icl_found_resource_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	cb_container->cb = cb;
	cb_container->user_data = user_data;

	sub_id = icl_dbus_subscribe_signal(signal_name, cb_container, free,
			_icl_found_resource_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		return IOTCON_ERROR_DBUS;
	}

	cb_container->id = sub_id;

	return ret;
}


/* If you know the information of resource, then you can make a proxy of the resource. */
API int iotcon_client_create(const char *host,
		const char *uri_path,
		bool is_observable,
		iotcon_resource_types_h resource_types,
		int resource_ifs,
		iotcon_client_h *client_handle)
{
	FN_CALL;
	iotcon_client_h resource = NULL;

	RETV_IF(NULL == host, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == client_handle, IOTCON_ERROR_INVALID_PARAMETER);

	resource = calloc(1, sizeof(struct icl_remote_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	resource->host = ic_utils_strdup(host);
	resource->uri_path = ic_utils_strdup(uri_path);
	resource->is_observable = is_observable;
	resource->types = icl_resource_types_ref(resource_types);
	resource->ifaces = resource_ifs;

	resource->ref_count = 1;

	*client_handle = resource;

	return IOTCON_ERROR_NONE;
}


API void iotcon_client_destroy(iotcon_client_h resource)
{
	RET_IF(NULL == resource);

	resource->ref_count--;

	if (0 < resource->ref_count)
		return;

	free(resource->uri_path);
	free(resource->host);
	free(resource->sid);
	iotcon_resource_types_destroy(resource->types);

	/* null COULD be allowed */
	if (resource->header_options)
		iotcon_options_destroy(resource->header_options);

	free(resource);
}


API int iotcon_client_ref(iotcon_client_h src, iotcon_client_h *dest)
{
	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(src->ref_count <= 0, IOTCON_ERROR_INVALID_PARAMETER);

	src->ref_count++;

	*dest = src;

	return IOTCON_ERROR_NONE;
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


/* The content of the resource should not be freed by user. */
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
		iotcon_options_destroy(resource->header_options);

	if (header_options)
		resource->header_options = icl_options_ref(header_options);
	else
		resource->header_options = NULL;

	return IOTCON_ERROR_NONE;
}


static iotcon_client_h _icl_client_from_gvariant(GVariant *payload,
		iotcon_connectivity_type_e conn_type)
{
	int ret;
	iotcon_client_h client;
	GVariantIter *types_iter;
	char host_addr[PATH_MAX] = {0};
	iotcon_resource_types_h res_types;
	char *uri_path, *sid, *res_type, *addr;
	int ifaces, is_observable, is_secure, port;

	g_variant_get(payload, "(&s&siasib&si)", &uri_path, &sid, &ifaces, &types_iter,
			&is_observable, &is_secure, &addr, &port);

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV6:
		snprintf(host_addr, sizeof(host_addr), "[%s]:%d", addr, port);
		break;
	case IOTCON_CONNECTIVITY_IPV4:
	default:
		snprintf(host_addr, sizeof(host_addr), "%s:%d", addr, port);
	}

	ret = iotcon_resource_types_create(&res_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		return NULL;
	}

	while (g_variant_iter_loop(types_iter, "s", &res_type))
		iotcon_resource_types_insert(res_types, res_type);

	ret = iotcon_client_create(host_addr, uri_path, !!is_observable, res_types, ifaces,
			&client);
	if (res_types)
		iotcon_resource_types_destroy(res_types);

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_create() Fail");
		return NULL;
	}
	client->ref_count = 1;

	client->sid = strdup(sid);
	if (NULL == client->sid) {
		ERR("strdup(sid) Fail(%d)", errno);
		iotcon_client_destroy(client);
		return NULL;
	}
	client->conn_type = conn_type;
	client->is_secure = is_secure;

	return client;
}

