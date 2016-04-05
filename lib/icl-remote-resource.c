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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <glib.h>

#include "iotcon-types.h"
#include "iotcon-internal.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-options.h"
#include "icl-dbus.h"
#include "icl-representation.h"
#include "icl-remote-resource.h"
#include "icl-resource-types.h"
#include "icl-resource-interfaces.h"
#include "icl-payload.h"

#include "icl-ioty.h"

#define ICL_REMOTE_RESOURCE_MAX_TIME_INTERVAL 3600 /* 60 min */

typedef struct {
	bool found;
	iotcon_found_resource_cb cb;
	void *user_data;
	unsigned int id;
	int timeout_id;
} icl_found_resource_s;

static iotcon_remote_resource_h _icl_remote_resource_from_gvariant(GVariant *payload,
		int connectivity_type);

static void _icl_found_resource_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	int connectivity_type;
	iotcon_remote_resource_h resource;

	GVariant *payload;
	icl_found_resource_s *cb_container = user_data;
	iotcon_found_resource_cb cb = cb_container->cb;

	cb_container->found = true;

	g_variant_get(parameters, "(vi)", &payload, &connectivity_type);

	resource = _icl_remote_resource_from_gvariant(payload, connectivity_type);
	if (NULL == resource) {
		ERR("icl_remote_resource_from_gvariant() Fail");
		return;
	}

	resource->is_found = true;

	if (cb)
		cb(resource, IOTCON_ERROR_NONE, cb_container->user_data);

	iotcon_remote_resource_destroy(resource);
}

static gboolean _icl_timeout_find_resource(gpointer p)
{
	icl_found_resource_s *cb_container = p;

	if (NULL == cb_container) {
		ERR("cb_container is NULL");
		return G_SOURCE_REMOVE;
	}

	if (false == cb_container->found && cb_container->cb)
		cb_container->cb(NULL, IOTCON_ERROR_TIMEOUT, cb_container->user_data);
	cb_container->timeout_id = 0;

	icl_dbus_unsubscribe_signal(cb_container->id);
	cb_container->id = 0;

	return G_SOURCE_REMOVE;
}

static void _icl_find_resource_conn_cleanup(icl_found_resource_s *cb_container)
{
	RET_IF(NULL == cb_container);
	if (cb_container->timeout_id)
		g_source_remove(cb_container->timeout_id);
	free(cb_container);
}


/* The length of resource_type should be less than or equal to 61.
 * If resource_type is NULL, then All resources in host are discovered. */
API int iotcon_find_resource(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *resource_type,
		bool is_secure,
		iotcon_found_resource_cb cb,
		void *user_data)
{
	unsigned int sub_id;
	GError *error = NULL;
	int64_t signal_number;
	int ret, timeout, conn_type;
	icl_found_resource_s *cb_container;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	if (resource_type && (ICL_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type))) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = icl_check_connectivity_type(connectivity_type, icl_get_service_mode());
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_check_connectivity_type() Fail(%d)", ret);
		return ret;
	}

	conn_type = connectivity_type;

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
	case IOTCON_CONNECTIVITY_IPV6:
	case IOTCON_CONNECTIVITY_ALL:
		ret = icl_ioty_find_resource(host_address, connectivity_type, resource_type,
				is_secure, cb, user_data);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_find_resource() Fail(%d)", ret);
			return ret;
		}
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	case IOTCON_CONNECTIVITY_BT_LE:
	case IOTCON_CONNECTIVITY_BT_ALL:
		RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
		timeout = icl_dbus_get_timeout();

		ic_dbus_call_find_resource_sync(icl_dbus_get_object(),
				ic_utils_dbus_encode_str(host_address),
				connectivity_type,
				ic_utils_dbus_encode_str(resource_type),
				is_secure,
				timeout,
				&signal_number,
				&ret,
				NULL,
				&error);
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

		snprintf(signal_name, sizeof(signal_name), "%s_%"PRIx64, IC_DBUS_SIGNAL_FOUND_RESOURCE,
				signal_number);

		cb_container = calloc(1, sizeof(icl_found_resource_s));
		if (NULL == cb_container) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		cb_container->cb = cb;
		cb_container->user_data = user_data;

		sub_id = icl_dbus_subscribe_signal(signal_name, cb_container,
				_icl_find_resource_conn_cleanup, _icl_found_resource_cb);
		if (0 == sub_id) {
			ERR("icl_dbus_subscribe_signal() Fail");
			return IOTCON_ERROR_DBUS;
		}
		cb_container->id = sub_id;

		cb_container->timeout_id = g_timeout_add_seconds(timeout, _icl_timeout_find_resource,
				cb_container);
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", conn_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

/* If you know the information of resource, then you can make a proxy of the resource. */
API int iotcon_remote_resource_create(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *uri_path,
		int properties,
		iotcon_resource_types_h resource_types,
		iotcon_resource_interfaces_h resource_ifaces,
		iotcon_remote_resource_h *resource_handle)
{
	FN_CALL;
	iotcon_remote_resource_h resource = NULL;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_ifaces, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_handle, IOTCON_ERROR_INVALID_PARAMETER);

	resource = calloc(1, sizeof(struct icl_remote_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	resource->host_address = ic_utils_strdup(host_address);
	resource->connectivity_type = connectivity_type;
	resource->uri_path = ic_utils_strdup(uri_path);
	resource->properties = properties;
	resource->types = icl_resource_types_ref(resource_types);
	resource->ifaces = icl_resource_interfaces_ref(resource_ifaces);
	resource->ref_count = 1;

	*resource_handle = resource;

	return IOTCON_ERROR_NONE;
}

static void _icl_remote_resource_destroy(iotcon_remote_resource_h resource)
{
	RET_IF(NULL == resource);

	if (resource->ref_count < 0) {
		ERR("Invalid ref_count (%d)", resource->ref_count);
		return;
	}

	if (true == resource->is_found) {
		ERR("It can't be destroyed by user.");
		return;
	}

	free(resource->uri_path);
	free(resource->host_address);
	free(resource->device_id);
	iotcon_resource_interfaces_destroy(resource->ifaces);
	iotcon_resource_types_destroy(resource->types);

	/* null COULD be allowed */
	if (resource->header_options)
		iotcon_options_destroy(resource->header_options);

	free(resource);
}

void icl_remote_resource_ref(iotcon_remote_resource_h resource)
{
	RET_IF(NULL == resource);
	resource->ref_count++;
}

void icl_remote_resource_unref(iotcon_remote_resource_h resource)
{
	RET_IF(NULL == resource);
	resource->ref_count--;
	if (0 == resource->ref_count)
		_icl_remote_resource_destroy(resource);
}


API void iotcon_remote_resource_destroy(iotcon_remote_resource_h resource)
{
	RET_IF(NULL == resource);

	if (resource->observe_handle)
		iotcon_remote_resource_observe_deregister(resource);
	icl_remote_resource_crud_stop(resource);
	if (0 != resource->caching_sub_id)
		iotcon_remote_resource_stop_caching(resource);
	if (0 != resource->monitoring_sub_id)
		iotcon_remote_resource_stop_monitoring(resource);

	icl_remote_resource_unref(resource);
}

static bool _icl_remote_resource_header_foreach_cb(unsigned short id,
		const char *data, void *user_data)
{
	int ret;
	iotcon_remote_resource_h resource = user_data;

	RETV_IF(NULL == resource, IOTCON_FUNC_STOP);

	if (NULL == resource->header_options) {
		ret = iotcon_options_create(&resource->header_options);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("resource->header_options() Fail(%d)", ret);
			return IOTCON_FUNC_STOP;
		}
	}

	ret = iotcon_options_add(resource->header_options, id, data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_options_add() Fail(%d)", ret);
		return IOTCON_FUNC_STOP;
	}

	return IOTCON_FUNC_CONTINUE;
}

API int iotcon_remote_resource_clone(iotcon_remote_resource_h src,
		iotcon_remote_resource_h *dest)
{
	int ret;
	iotcon_remote_resource_h resource = NULL;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	resource = calloc(1, sizeof(struct icl_remote_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	resource->uri_path = ic_utils_strdup(src->uri_path);
	resource->host_address = ic_utils_strdup(src->host_address);
	resource->connectivity_type = src->connectivity_type;
	resource->device_id = ic_utils_strdup(src->device_id);
	resource->properties = src->properties;
	resource->ref_count = 1;

	if (src->header_options) {
		ret = iotcon_options_foreach(src->header_options,
				_icl_remote_resource_header_foreach_cb, resource);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_options_foreach() Fail(%d)", ret);
			iotcon_remote_resource_destroy(resource);
			return ret;
		}
	}

	ret = iotcon_resource_types_clone(src->types, &resource->types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_clone() Fail(%d)", ret);
		iotcon_remote_resource_destroy(resource);
		return ret;
	}

	ret = iotcon_resource_interfaces_clone(src->ifaces, &resource->ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_interfaces_clone() Fail(%d)", ret);
		iotcon_remote_resource_destroy(resource);
		return ret;
	}

	resource->connectivity_type = src->connectivity_type;

	*dest = resource;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_remote_resource_get_uri_path(iotcon_remote_resource_h resource,
		char **uri_path)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = resource->uri_path;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_remote_resource_get_host_address(iotcon_remote_resource_h resource,
		char **host_address)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);

	*host_address = resource->host_address;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_remote_resource_get_connectivity_type(iotcon_remote_resource_h resource,
		iotcon_connectivity_type_e *connectivity_type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == connectivity_type, IOTCON_ERROR_INVALID_PARAMETER);

	*connectivity_type = resource->connectivity_type;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_remote_resource_get_device_id(iotcon_remote_resource_h resource,
		char **device_id)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device_id, IOTCON_ERROR_INVALID_PARAMETER);

	*device_id = resource->device_id;

	return IOTCON_ERROR_NONE;
}

/* The content of the resource should not be freed by user. */
API int iotcon_remote_resource_get_types(iotcon_remote_resource_h resource,
		iotcon_resource_types_h *types)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = resource->types;

	return IOTCON_ERROR_NONE;
}


/* The content of the resource should not be freed by user. */
API int iotcon_remote_resource_get_interfaces(iotcon_remote_resource_h resource,
		iotcon_resource_interfaces_h *ifaces)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = resource->ifaces;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_get_properties(iotcon_remote_resource_h resource,
		int *properties)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == properties, IOTCON_ERROR_INVALID_PARAMETER);

	*properties = resource->properties;

	return IOTCON_ERROR_NONE;
}

API int iotcon_remote_resource_get_options(iotcon_remote_resource_h resource,
		iotcon_options_h *options)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER);

	*options = resource->header_options;

	return IOTCON_ERROR_NONE;
}

/* if header_options is NULL, then client's header_options is unset */
API int iotcon_remote_resource_set_options(iotcon_remote_resource_h resource,
		iotcon_options_h options)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (options)
		options = icl_options_ref(options);

	if (resource->header_options)
		iotcon_options_destroy(resource->header_options);

	resource->header_options = options;

	return IOTCON_ERROR_NONE;
}


static iotcon_remote_resource_h _icl_remote_resource_from_gvariant(GVariant *payload,
		int connectivity_type)
{
	int ret;
	iotcon_remote_resource_h resource;
	GVariantIter *types_iter, *ifaces_iter;
	char host_addr[PATH_MAX] = {0};
	iotcon_resource_types_h res_types;
	iotcon_resource_interfaces_h ifaces;
	char *uri_path, *device_id, *res_type, *iface, *addr;
	int properties, is_secure, port;

	g_variant_get(payload, "(&s&sasasib&si)", &uri_path, &device_id, &ifaces_iter,
			&types_iter, &properties, &is_secure, &addr, &port);

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV6:
		snprintf(host_addr, sizeof(host_addr), "[%s]:%d", addr, port);
		break;
	case IOTCON_CONNECTIVITY_IPV4:
		snprintf(host_addr, sizeof(host_addr), "%s:%d", addr, port);
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
	default:
		snprintf(host_addr, sizeof(host_addr), "%s", addr);
	}

	ret = iotcon_resource_types_create(&res_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		return NULL;
	}

	while (g_variant_iter_loop(types_iter, "s", &res_type))
		iotcon_resource_types_add(res_types, res_type);

	ret = iotcon_resource_interfaces_create(&ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_interfaces_create() Fail(%d)", ret);
		iotcon_resource_types_destroy(res_types);
		return NULL;
	}

	while (g_variant_iter_loop(ifaces_iter, "s", &iface))
		iotcon_resource_interfaces_add(ifaces, iface);

	ret = iotcon_remote_resource_create(host_addr, connectivity_type, uri_path,
			properties, res_types, ifaces, &resource);

	iotcon_resource_interfaces_destroy(ifaces);
	iotcon_resource_types_destroy(res_types);

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remote_resource_create() Fail");
		return NULL;
	}

	resource->device_id = strdup(device_id);
	if (NULL == resource->device_id) {
		ERR("strdup(device_id) Fail(%d)", errno);
		iotcon_remote_resource_destroy(resource);
		return NULL;
	}
	resource->connectivity_type = connectivity_type;
	resource->properties = properties;

	return resource;
}


API int iotcon_remote_resource_get_time_interval(int *time_interval)
{
	GError *error = NULL;
	int ret, arg_time_interval;
	iotcon_service_mode_e mode;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == time_interval, IOTCON_ERROR_INVALID_PARAMETER);

	mode = icl_get_service_mode();

	if (IOTCON_SERVICE_IP & mode) {
		ret = icl_ioty_remote_resource_get_time_interval(&arg_time_interval);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_get_time_interval() Fail(%d)", ret);
			return ret;
		}
	} else if (IOTCON_SERVICE_BT & mode) {
		ic_dbus_call_encap_get_time_interval_sync(icl_dbus_get_object(), &arg_time_interval,
				NULL, &error);
		if (error) {
			ERR("ic_dbus_call_encap_get_time_interval_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			return ret;
		}
	} else {
		ERR("Invalid Mode(%d)", mode);
		return IOTCON_ERROR_SYSTEM;
	}

	*time_interval = arg_time_interval;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_set_time_interval(int time_interval)
{
	int ret;
	iotcon_service_mode_e mode;
	GError *error = NULL;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(ICL_REMOTE_RESOURCE_MAX_TIME_INTERVAL < time_interval || time_interval <= 0,
			IOTCON_ERROR_INVALID_PARAMETER);

	mode = icl_get_service_mode();

	if (IOTCON_SERVICE_IP & mode) {
		ret = icl_ioty_remote_resource_set_time_interval(time_interval);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_remote_resource_set_time_interval() Fail(%d)", ret);
			return ret;
		}
	}
	if (IOTCON_SERVICE_BT & mode) {
		ic_dbus_call_encap_set_time_interval_sync(icl_dbus_get_object(), time_interval,
				NULL, &error);
		if (error) {
			ERR("ic_dbus_call_encap_set_time_interval_sync() Fail(%s)", error->message);
			ret = icl_dbus_convert_dbus_error(error->code);
			g_error_free(error);
			return ret;
		}
	}

	return IOTCON_ERROR_NONE;
}

