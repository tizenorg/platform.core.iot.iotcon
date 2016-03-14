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
#include <stdlib.h>
#include <glib.h>
#include <octypes.h>
#include <ocstack.h>
#include <ocpayload.h>
#include <system_info.h>
#include <system_settings.h>

#include "iotcon.h"
#include "iotcon-internal.h"
#include "ic-utils.h"
#include "ic-ioty-types.h"
#include "icl.h"
#include "icl-dbus.h"
#include "icl-types.h"
#include "icl-presence.h"
#include "icl-remote-resource.h"
#include "icl-representation.h"
#include "icl-query.h"
#include "icl-request.h"
#include "icl-resource.h"
#include "icl-resource-interfaces.h"
#include "icl-resource-types.h"
#include "icl-response.h"
#include "icl-observation.h"
#include "icl-state.h"
#include "icl-lite-resource.h"
#include "icl-ioty-ocprocess.h"
#include "icl-ioty-types.h"
#include "icl-ioty.h"

static const char *ICL_SYSTEM_INFO_PLATFORM_NAME = "http://tizen.org/system/platform.name";
static const char *ICL_SYSTEM_INFO_PLATFORM_VERSION = "http://tizen.org/feature/platform.version";
static const char *ICL_SYSTEM_INFO_MANUF_NAME = "http://tizen.org/system/manufacturer";
static const char *ICL_SYSTEM_INFO_MODEL_NAME = "http://tizen.org/system/model_name";
static const char *ICL_SYSTEM_INFO_BUILD_STRING = "http://tizen.org/system/build.string";

static GMutex icl_csdk_mutex;
static int icl_remote_resource_time_interval = IC_REMOTE_RESOURCE_DEFAULT_TIME_INTERVAL;
static GHashTable *icl_monitoring_table;
static GHashTable *icl_caching_table;

void icl_ioty_csdk_lock()
{
	g_mutex_lock(&icl_csdk_mutex);
}

void icl_ioty_csdk_unlock()
{
	g_mutex_unlock(&icl_csdk_mutex);
}

void icl_ioty_deinit(GThread *thread)
{
	FN_CALL;
	OCStackResult result;

	icl_ioty_ocprocess_stop();
	g_thread_join(thread);

	result = OCStop();
	if (OC_STACK_OK != result)
		ERR("OCStop() Fail(%d)", result);
}

int icl_ioty_init(GThread **out_thread)
{
	FN_CALL;
	GError *error;
	GThread *thread;

	OCStackResult result = OCInit(NULL, 0, OC_CLIENT_SERVER);
	if (OC_STACK_OK != result) {
		ERR("OCInit() Fail(%d)", result);
		return ic_ioty_parse_oic_error(result);
	}

	icl_ioty_ocprocess_start();

	thread = g_thread_try_new("packet_receive_thread", icl_ioty_ocprocess_thread,
			NULL, &error);
	if (NULL == thread) {
		ERR("g_thread_try_new() Fail(%s)", error->message);
		g_error_free(error);
		return IOTCON_ERROR_SYSTEM;
	}

	*out_thread = thread;

	return IOTCON_ERROR_NONE;
}

static gboolean _icl_ioty_timeout(gpointer user_data)
{
	FN_CALL;
	int ret;
	icl_cb_s *cb_info = user_data;

	RETV_IF(NULL == cb_info, G_SOURCE_REMOVE);
	cb_info->timeout = 0;

	if (false == cb_info->found && cb_info->cb) {
		switch (cb_info->op) {
		case ICL_FIND_RESOURCE:
			((iotcon_found_resource_cb)cb_info->cb)(NULL, IOTCON_ERROR_TIMEOUT,
					cb_info->user_data);
			break;
		case ICL_GET_DEVICE_INFO:
			((iotcon_device_info_cb)cb_info->cb)(NULL, IOTCON_ERROR_TIMEOUT,
					cb_info->user_data);
			break;
		case ICL_GET_PLATFORM_INFO:
			((iotcon_platform_info_cb)cb_info->cb)(NULL, IOTCON_ERROR_TIMEOUT,
					cb_info->user_data);
			break;
		default:
			ERR("Invalid operation(%d)", cb_info->op);
			return G_SOURCE_REMOVE;
		}
	}

	icl_ioty_csdk_lock();
	ret = OCCancel(cb_info->handle, OC_LOW_QOS, NULL, 0);
	icl_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCCancel() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}

	return G_SOURCE_REMOVE;
}

static void _icl_ioty_free_cb_data(void *data)
{
	icl_cb_s *cb_info = data;
	RET_IF(NULL == cb_info);

	if (cb_info->timeout)
		g_source_remove(cb_info->timeout);

	free(cb_info);
}

int icl_ioty_find_resource(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *resource_type,
		bool is_secure,
		iotcon_found_resource_cb cb,
		void *user_data)
{
	FN_CALL;
	int len, timeout;
	char *coap_str;
	char uri[PATH_MAX] = {0};
	icl_cb_s *cb_data;
	OCDoHandle handle;
	OCStackResult result;
	OCCallbackData cbdata = {0};
	OCConnectivityType oic_conn_type;

	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	coap_str = is_secure ? IC_IOTY_COAPS : IC_IOTY_COAP;

	if (NULL == host_address) {
		len = snprintf(uri, sizeof(uri), "%s", OC_RSRVD_WELL_KNOWN_URI);
	} else {
		len = snprintf(uri, sizeof(uri), "%s%s%s", coap_str, host_address,
				OC_RSRVD_WELL_KNOWN_URI);
	}
	if (len <= 0 || sizeof(uri) <= len) {
		ERR("snprintf() Fail(%d)", len);
		return IOTCON_ERROR_IO_ERROR;
	}

	if (resource_type)
		snprintf(uri + len, sizeof(uri) - len, "?rt=%s", resource_type);

	cb_data = calloc(1, sizeof(icl_cb_s));
	if (NULL == cb_data) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_data->op = ICL_FIND_RESOURCE;
	cb_data->cb = cb;
	cb_data->user_data = user_data;

	cbdata.context = cb_data;
	cbdata.cb = icl_ioty_ocprocess_find_cb;
	cbdata.cd = _icl_ioty_free_cb_data;

	oic_conn_type = ic_ioty_convert_connectivity_type(connectivity_type);

	icl_ioty_csdk_lock();
	/* TODO : QoS is come from lib. */
	result = OCDoResource(&handle, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();
	cb_data->handle = handle;

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		_icl_ioty_free_cb_data(cb_data);
		return ic_ioty_parse_oic_error(result);
	}

	iotcon_get_timeout(&timeout);
	cb_data->timeout = g_timeout_add_seconds(timeout, _icl_ioty_timeout, cb_data);

	return IOTCON_ERROR_NONE;
}

int icl_ioty_get_device_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_device_info_cb cb,
		void *user_data)
{
	int timeout;
	char uri[PATH_MAX] = {0};
	icl_cb_s *cb_data = NULL;
	OCDoHandle handle;
	OCStackResult result;
	OCCallbackData cbdata = {0};
	OCConnectivityType oic_conn_type;

	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == host_address)
		snprintf(uri, sizeof(uri), "%s", OC_RSRVD_DEVICE_URI);
	else
		snprintf(uri, sizeof(uri), "%s%s", host_address, OC_RSRVD_DEVICE_URI);

	cb_data = calloc(1, sizeof(icl_cb_s));
	if (NULL == cb_data) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_data->cb = cb;
	cb_data->user_data = user_data;
	cb_data->op = ICL_GET_DEVICE_INFO;

	cbdata.context = cb_data;
	cbdata.cb = icl_ioty_ocprocess_device_info_cb;
	cbdata.cd = _icl_ioty_free_cb_data;

	oic_conn_type = ic_ioty_convert_connectivity_type(connectivity_type);

	icl_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	result = OCDoResource(&handle, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();
	cb_data->handle = handle;

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		_icl_ioty_free_cb_data(cb_data);
		return ic_ioty_parse_oic_error(result);
	}

	iotcon_get_timeout(&timeout);
	cb_data->timeout = g_timeout_add_seconds(timeout, _icl_ioty_timeout, cb_data);

	return IOTCON_ERROR_NONE;
}

int icl_ioty_get_platform_info(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		iotcon_platform_info_cb cb,
		void *user_data)
{
	int timeout;
	char uri[PATH_MAX] = {0};
	icl_cb_s *cb_data = NULL;
	OCDoHandle handle;
	OCStackResult result;
	OCCallbackData cbdata = {0};
	OCConnectivityType oic_conn_type;

	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == host_address)
		snprintf(uri, sizeof(uri), "%s", OC_RSRVD_PLATFORM_URI);
	else
		snprintf(uri, sizeof(uri), "%s%s", host_address, OC_RSRVD_PLATFORM_URI);

	cb_data = calloc(1, sizeof(icl_cb_s));
	if (NULL == cb_data) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_data->cb = cb;
	cb_data->user_data = user_data;
	cb_data->op = ICL_GET_PLATFORM_INFO;

	cbdata.context = cb_data;
	cbdata.cb = icl_ioty_ocprocess_platform_info_cb;
	cbdata.cd = _icl_ioty_free_cb_data;

	oic_conn_type = ic_ioty_convert_connectivity_type(connectivity_type);

	icl_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	result = OCDoResource(&handle, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();
	cb_data->handle = handle;

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		_icl_ioty_free_cb_data(cb_data);
		return ic_ioty_parse_oic_error(result);
	}

	iotcon_get_timeout(&timeout);
	cb_data->timeout = g_timeout_add_seconds(timeout, _icl_ioty_timeout, cb_data);

	return IOTCON_ERROR_NONE;
}

static int _ioty_set_device_info()
{
	int ret;
	char *device_name = NULL;
	OCDeviceInfo device_info = {0};

	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_DEVICE_NAME, &device_name);
	if (SYSTEM_SETTINGS_ERROR_NONE != ret) {
		ERR("system_settings_get_value_string() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}

	device_info.deviceName = device_name;

	icl_ioty_csdk_lock();
	ret = OCSetDeviceInfo(device_info);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCSetDeviceInfo() Fail(%d)", ret);
		free(device_name);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

static void _icl_ioty_on_device_name_changed_cb(system_settings_key_e key,
		void *user_data)
{
	int ret;

	ret = _ioty_set_device_info();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ioty_set_device_info() Fail(%d)", ret);
		return;
	}
}

int icl_ioty_set_device_info()
{
	int ret;

	ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_DEVICE_NAME,
			_icl_ioty_on_device_name_changed_cb, NULL);
	if (SYSTEM_SETTINGS_ERROR_NONE != ret) {
		ERR("system_settings_set_changed_cb() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = _ioty_set_device_info();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ioty_set_device_info() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}

static void _ioty_free_platform_info(OCPlatformInfo platform_info)
{
	free(platform_info.manufacturerName);
	free(platform_info.manufacturerUrl);
	free(platform_info.modelNumber);
	free(platform_info.dateOfManufacture);
	free(platform_info.platformVersion);
	free(platform_info.operatingSystemVersion);
	free(platform_info.hardwareVersion);
	free(platform_info.firmwareVersion);
	free(platform_info.supportUrl);
	free(platform_info.systemTime);
}

int icl_ioty_set_platform_info()
{
	int ret;
	OCPlatformInfo platform_info = {0};

	ret = system_info_get_platform_string(ICL_SYSTEM_INFO_PLATFORM_NAME,
			&platform_info.platformID);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string() Fail(%d)", ret);
		_ioty_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = system_info_get_platform_string(ICL_SYSTEM_INFO_MANUF_NAME,
			&platform_info.manufacturerName);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string() Fail(%d)", ret);
		_ioty_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = system_info_get_platform_string(ICL_SYSTEM_INFO_MODEL_NAME,
			&platform_info.modelNumber);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string() Fail(%d)", ret);
		_ioty_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = system_info_get_platform_string(ICL_SYSTEM_INFO_PLATFORM_VERSION,
			&platform_info.platformVersion);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string() Fail(%d)", ret);
		_ioty_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = system_info_get_platform_string(ICL_SYSTEM_INFO_BUILD_STRING,
			&platform_info.firmwareVersion);
	if (SYSTEM_INFO_ERROR_NONE != ret) {
		ERR("system_info_get_platform_string() Fail(%d)", ret);
		_ioty_free_platform_info(platform_info);
		return IOTCON_ERROR_SYSTEM;
	}

	/* platform_info.manufacturerUrl */
	/* platform_info.dateOfManufacture */
	/* platform_info.operatingSystemVersion */
	/* platform_info.hardwareVersion */
	/* platform_info.supportUrl */
	/* platform_info.systemTime */

	icl_ioty_csdk_lock();
	ret = OCSetPlatformInfo(platform_info);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCSetPlatformInfo() Fail(%d)", ret);
		_ioty_free_platform_info(platform_info);
		return ic_ioty_parse_oic_error(ret);
	}
	_ioty_free_platform_info(platform_info);

	return IOTCON_ERROR_NONE;
}


int icl_ioty_add_presence_cb(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *resource_type,
		iotcon_presence_cb cb,
		void *user_data,
		iotcon_presence_h *presence_handle)
{
	OCDoHandle handle;
	const char *address;
	OCStackResult result;
	char uri[PATH_MAX] = {0};
	OCCallbackData cbdata = {0};
	OCConnectivityType oic_conn_type;
	iotcon_presence_h presence = NULL;

	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == presence_handle, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == host_address)
		address = IC_IOTY_MULTICAST_ADDRESS;
	else
		address = host_address;

	snprintf(uri, sizeof(uri), "%s%s", address, OC_RSRVD_PRESENCE_URI);

	presence = calloc(1, sizeof(struct icl_presence));
	if (NULL == presence) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	presence->cb = cb;
	presence->user_data = user_data;
	if (host_address)
		presence->host_address = strdup(host_address);
	presence->connectivity_type = connectivity_type;
	if (resource_type)
		presence->resource_type = strdup(resource_type);

	cbdata.context = presence;
	cbdata.cd = NULL;
	cbdata.cb = icl_ioty_ocprocess_presence_cb;

	oic_conn_type = ic_ioty_convert_connectivity_type(connectivity_type);

	icl_ioty_csdk_lock();
	result = OCDoResource(&handle, OC_REST_PRESENCE, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();
	presence->handle = IC_POINTER_TO_INT64(handle);

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		icl_destroy_presence(presence);
		return IOTCON_ERROR_IOTIVITY;
	}

	*presence_handle = presence;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_remove_presence_cb(iotcon_presence_h presence)
{
	int ret;

	RETV_IF(NULL == presence, IOTCON_ERROR_INVALID_PARAMETER);

	icl_ioty_csdk_lock();
	ret = OCCancel(IC_INT64_TO_POINTER(presence->handle), OC_LOW_QOS, NULL, 0);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCCancel() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	icl_destroy_presence(presence);

	return IOTCON_ERROR_NONE;
}

static char* _icl_ioty_resource_generate_uri(char *uri_path, GHashTable *hash)
{
	int len;
	bool loop_first = true;
	char uri_buf[PATH_MAX] = {0};
	GHashTableIter iter;
	gpointer key, value;

	RETV_IF(NULL == uri_path, NULL);

	len = snprintf(uri_buf, sizeof(uri_buf), "%s", uri_path);

	/* remove suffix '/' */
	if ('/' == uri_buf[strlen(uri_buf) - 1]) {
		uri_buf[strlen(uri_buf) - 1] = '\0';
		len--;
	}

	if (hash) {
		g_hash_table_iter_init(&iter, hash);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			int query_len;
			DBG("query exist. key(%s), value(%s)", key, value);

			if (true == loop_first) {
				query_len = snprintf(uri_buf + len, sizeof(uri_buf) - len, "?%s=%s",
						(char *)key, (char *)value);
				loop_first = false;
			} else {
				query_len = snprintf(uri_buf + len, sizeof(uri_buf) - len, ";%s=%s",
						(char *)key, (char *)value);
			}
			len += query_len;
		}
	}

	return strdup(uri_buf);
}

static void _icl_ioty_free_observe_container(void *data)
{
	icl_observe_container_s *cb_container = data;
	icl_remote_resource_unref(cb_container->resource);
	free(cb_container);
}


static int _icl_ioty_remote_resource_observe(iotcon_remote_resource_h resource,
		iotcon_observe_policy_e observe_policy,
		iotcon_query_h query,
		iotcon_remote_resource_observe_cb cb,
		void *user_data,
		OCDoHandle *observe_handle)
{
	int ret, options_size = 0;
	char *uri = NULL;
	OCMethod method;
	OCDoHandle handle;
	OCDevAddr dev_addr = {0};
	OCCallbackData cbdata = {0};
	OCConnectivityType oic_conn_type;
	OCHeaderOption *oic_options_ptr = NULL;
	OCHeaderOption oic_options[MAX_HEADER_OPTIONS];
	GHashTable *query_hash;
	icl_observe_container_s *cb_container;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	/* uri */
	query_hash = (query && query->hash) ? query->hash : NULL;
	uri = _icl_ioty_resource_generate_uri(resource->uri_path, query_hash);
	if (NULL == uri) {
		ERR("_icd_ioty_resource_generate_uri() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	/* method */
	switch (observe_policy) {
	case IOTCON_OBSERVE_IGNORE_OUT_OF_ORDER:
		method = OC_REST_OBSERVE;
		break;
	case IOTCON_OBSERVE_ACCEPT_OUT_OF_ORDER:
	default:
		method = OC_REST_OBSERVE_ALL;
	}

	/* connectivity type */
	oic_conn_type = ic_ioty_convert_connectivity_type(resource->connectivity_type);

	/* host address, port */
	ret = ic_ioty_convert_connectivity(resource->host_address,
			resource->connectivity_type, &dev_addr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_convert_connectivity() Fail(%d)", ret);
		free(uri);
		return ret;
	}

	/* options */
	if (resource->header_options && resource->header_options->hash) {
		ret = icl_ioty_convert_header_options(resource->header_options, oic_options,
				sizeof(oic_options)/sizeof(oic_options[0]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_convert_header_options() Fail(%d)", ret);
			free(uri);
			return ret;
		}
		options_size = g_hash_table_size(resource->header_options->hash);
	}

	cb_container = calloc(1, sizeof(icl_observe_container_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail");
		free(uri);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_container->cb = cb;
	cb_container->user_data = user_data;
	cb_container->resource = resource;
	icl_remote_resource_ref(resource);

	cbdata.context = cb_container;
	cbdata.cb = icl_ioty_ocprocess_observe_cb;
	cbdata.cd = _icl_ioty_free_observe_container;

	icl_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	ret = OCDoResource(&handle, method, uri, &dev_addr, NULL, oic_conn_type,
			OC_HIGH_QOS, &cbdata, oic_options_ptr, options_size);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCDoResource() Fail(%d)", ret);
		_icl_ioty_free_observe_container(cb_container);
		free(uri);
		return IOTCON_ERROR_IOTIVITY;
	}

	free(uri);
	*observe_handle = handle;

	return IOTCON_ERROR_NONE;

}

int icl_ioty_remote_resource_observe_register(
		iotcon_remote_resource_h resource,
		iotcon_observe_policy_e observe_policy,
		iotcon_query_h query,
		iotcon_remote_resource_observe_cb cb,
		void *user_data)
{
	int ret;
	OCDoHandle handle = NULL;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(resource->observe_handle, IOTCON_ERROR_ALREADY);

	ret = _icl_ioty_remote_resource_observe(resource, observe_policy, query, cb, user_data, &handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_remote_resource_observe() Fail(%d)", ret);
		return ret;
	}
	resource->observe_handle = IC_POINTER_TO_INT64(handle);
	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_observe_cancel(iotcon_remote_resource_h resource,
		int64_t observe_handle)
{
	int ret;
	int options_size = 0;
	OCDoHandle handle;
	OCHeaderOption *oic_options_ptr = NULL;
	OCHeaderOption oic_options[MAX_HEADER_OPTIONS];

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	/* options */
	if (resource->header_options && resource->header_options->hash) {
		ret = icl_ioty_convert_header_options(resource->header_options, oic_options,
				sizeof(oic_options)/sizeof(oic_options[0]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_convert_header_options() Fail(%d)", ret);
			return ret;
		}
		options_size = g_hash_table_size(resource->header_options->hash);
	}
	handle = IC_INT64_TO_POINTER(observe_handle);

	icl_ioty_csdk_lock();
	ret = OCCancel(handle, OC_HIGH_QOS, oic_options_ptr, options_size);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCCancel() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_observe_deregister(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(0 == resource->observe_handle, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_ioty_remote_resource_observe_cancel(resource, resource->observe_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_remote_resource_observe_cancel() Fail(%d)", ret);
		return ret;
	}

	resource->observe_handle = 0;
	return IOTCON_ERROR_NONE;
}

static void _icl_ioty_free_response_container(void *data)
{
	icl_response_container_s *cb_container = data;
	RET_IF(NULL == cb_container);

	if (cb_container->timeout) {
		g_source_remove(cb_container->timeout);
		cb_container->timeout = 0;
	}

	icl_remote_resource_unref(cb_container->resource);
	free(cb_container);
}

static gboolean _icl_ioty_response_timeout(gpointer p)
{
	icl_response_container_s *cb_container = p;
	RETV_IF(NULL == cb_container, G_SOURCE_REMOVE);
	cb_container->timeout = 0;

	if (cb_container->cb) {
		cb_container->cb(cb_container->resource, IOTCON_ERROR_TIMEOUT,
				cb_container->req_type, NULL, cb_container->user_data);
	}

	return G_SOURCE_REMOVE;
}

static int _icl_ioty_remote_resource_crud(
		iotcon_request_type_e req_type,
		iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_response_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret, timeout, options_size = 0;
	char *uri;
	GHashTable *query_hash;
	icl_response_container_s *cb_container;
	OCCallbackData cbdata = {0};
	OCHeaderOption *oic_options_ptr = NULL;
	OCHeaderOption oic_options[MAX_HEADER_OPTIONS];
	OCConnectivityType oic_conn_type;
	OCDevAddr dev_addr = {0};
	OCMethod method;
	OCPayload *payload = NULL;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	/* method (request type) */
	method = ic_ioty_convert_request_type(req_type);

	/* uri */
	query_hash = (query && query->hash) ? query->hash : NULL;
	uri = _icl_ioty_resource_generate_uri(resource->uri_path, query_hash);
	if (NULL == uri) {
		ERR("_icl_ioty_resource_generate_uri() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	/* host address, port */
	ret = ic_ioty_convert_connectivity(resource->host_address,
			resource->connectivity_type, &dev_addr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_convert_connectivity() Fail(%d)", ret);
		free(uri);
		return ret;
	}

	/* representation */
	if (repr) {
		ret = icl_ioty_convert_representation(repr, &payload);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_convert_representation() Fail(%d)", ret);
			free(uri);
			return ret;
		}
	}

	/* connectivity type */
	oic_conn_type = ic_ioty_convert_connectivity_type(resource->connectivity_type);

	/* options */
	if (resource->header_options && resource->header_options->hash) {
		ret = icl_ioty_convert_header_options(resource->header_options, oic_options,
				sizeof(oic_options)/sizeof(oic_options[0]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_convert_header_options() Fail(%d)", ret);
			OCRepPayloadDestroy((OCRepPayload*)payload);
			free(uri);
			return ret;
		}
		options_size = g_hash_table_size(resource->header_options->hash);
	}

	cb_container = calloc(1, sizeof(icl_response_container_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		OCRepPayloadDestroy((OCRepPayload*)payload);
		free(uri);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_container->resource = resource;
	icl_remote_resource_ref(cb_container->resource);
	cb_container->req_type = req_type;
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	cbdata.context = cb_container;
	cbdata.cb = icl_ioty_ocprocess_crud_cb;
	cbdata.cd = _icl_ioty_free_response_container;

	icl_ioty_csdk_lock();
	ret = OCDoResource(NULL, method, uri, &dev_addr, payload, oic_conn_type,
			OC_HIGH_QOS, &cbdata, oic_options_ptr, options_size);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCDoResource() Fail(%d)", ret);
		free(uri);
		_icl_ioty_free_response_container(cb_container);
		return ic_ioty_parse_oic_error(ret);
	}

	free(uri);
	iotcon_get_timeout(&timeout);
	cb_container->timeout = g_timeout_add_seconds(timeout, _icl_ioty_response_timeout,
			cb_container);

	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_get(iotcon_remote_resource_h resource,
		iotcon_query_h query, iotcon_remote_resource_response_cb cb, void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	/* TODO: cynara check */

	ret = _icl_ioty_remote_resource_crud(IOTCON_REQUEST_GET, resource, NULL, query,
			cb, user_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_remote_resource_crud() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


int icl_ioty_remote_resource_put(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_response_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	/* TODO: cynara check */

	ret = _icl_ioty_remote_resource_crud(IOTCON_REQUEST_PUT, resource, repr, query,
			cb, user_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_remote_resource_crud() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_post(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_query_h query,
		iotcon_remote_resource_response_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	/* TODO: cynara check */

	ret = _icl_ioty_remote_resource_crud(IOTCON_REQUEST_POST, resource, repr, query,
			cb, user_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_remote_resource_crud() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_delete(iotcon_remote_resource_h resource,
		iotcon_remote_resource_response_cb cb,
		void *user_data)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	/* TODO: cynara check */

	ret = _icl_ioty_remote_resource_crud(IOTCON_REQUEST_DELETE, resource, NULL, NULL,
			cb, user_data);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_remote_resource_crud() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}

static void _icl_ioty_monitoring_get_cb(iotcon_remote_resource_h resource,
		iotcon_error_e err,
		iotcon_request_type_e request_type,
		iotcon_response_h response,
		void *user_data)
{
	iotcon_remote_resource_state_e state;
	icl_monitoring_container_s *cb_container = user_data;

	RET_IF(NULL == resource);
	RET_IF(NULL == response);

	state = (IOTCON_RESPONSE_OK == response->result) ?
			IOTCON_REMOTE_RESOURCE_ALIVE : IOTCON_REMOTE_RESOURCE_LOST_SIGNAL;

	if (state != cb_container->state) {
		cb_container->cb(cb_container->resource, state, cb_container->user_data);
		cb_container->state = state;
	}
}

static gboolean _icl_ioty_monitoring_idle_cb(gpointer p)
{
	int ret;
	icl_monitoring_container_s *cb_container = p;

	RETV_IF(NULL == cb_container, G_SOURCE_REMOVE);

	/* get */
	ret = icl_ioty_remote_resource_get(cb_container->resource, NULL,
			_icl_ioty_monitoring_get_cb, cb_container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_remote_resource_get() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}

	/* get timeout */
	if (cb_container->timeout)
		g_source_remove(cb_container->timeout);
	cb_container->timeout = g_timeout_add_seconds(icl_remote_resource_time_interval,
			_icl_ioty_monitoring_idle_cb, cb_container);

	return G_SOURCE_REMOVE;
}

static void _icl_ioty_monitoring_presence_cb(iotcon_presence_h presence,
		iotcon_error_e err, iotcon_presence_response_h response, void *user_data)
{
	icl_monitoring_container_s *cb_container = user_data;

	RET_IF(NULL == cb_container);

	if (IOTCON_PRESENCE_OK == response->result &&
			IOTCON_PRESENCE_RESOURCE_DESTROYED != response->trigger)
		return;

	g_idle_add(_icl_ioty_monitoring_idle_cb, cb_container);
}

static void _icl_ioty_remote_resource_monitoring_table_insert(
		iotcon_remote_resource_h resource, icl_monitoring_container_s *cb_container)
{
	RET_IF(NULL == resource);
	RET_IF(NULL == cb_container);

	if (NULL == icl_monitoring_table) {
		icl_monitoring_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
				icl_destroy_monitoring_container);
	}
	g_hash_table_insert(icl_monitoring_table, resource, cb_container);
}

static int icl_remote_resource_monitoring_table_remove(iotcon_remote_resource_h resource)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == icl_monitoring_table)
		return IOTCON_ERROR_NO_DATA;

	if (NULL == g_hash_table_lookup(icl_monitoring_table, resource))
		return IOTCON_ERROR_NO_DATA;

	g_hash_table_remove(icl_monitoring_table, resource);

	return IOTCON_ERROR_NONE;
}


int icl_ioty_remote_resource_start_monitoring(iotcon_remote_resource_h resource,
		iotcon_remote_resource_state_changed_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	iotcon_presence_h presence;
	icl_monitoring_container_s *cb_container;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = calloc(1, sizeof(icl_monitoring_container_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_container->state = IOTCON_REMOTE_RESOURCE_ALIVE;
	cb_container->resource = resource;
	icl_remote_resource_ref(cb_container->resource);
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	/* presence */
	ret = icl_ioty_add_presence_cb(resource->host_address, resource->connectivity_type,
			NULL, _icl_ioty_monitoring_presence_cb, cb_container, &presence);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_add_presence_cb() Fail(%d)", ret);
		icl_destroy_monitoring_container(cb_container);
		return ret;
	}
	cb_container->presence = presence;

	/* get */
	ret = icl_ioty_remote_resource_get(resource, NULL, _icl_ioty_monitoring_get_cb,
			cb_container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_remote_resource_get() Fail(%d)", ret);
		icl_destroy_monitoring_container(cb_container);
		return ret;
	}

	/* get timeout */
	cb_container->timeout = g_timeout_add_seconds(icl_remote_resource_time_interval,
			_icl_ioty_monitoring_idle_cb, cb_container);

	_icl_ioty_remote_resource_monitoring_table_insert(resource, cb_container);

	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_stop_monitoring(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_remote_resource_monitoring_table_remove(resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_remote_resource_monitoring_table_remove() Fail(%d)", ret);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

static void _icl_ioty_caching_get_cb(iotcon_remote_resource_h resource,
		iotcon_error_e err,
		iotcon_request_type_e request_type,
		iotcon_response_h response,
		void *user_data)
{
	FN_CALL;
	int ret;
	icl_caching_container_s *cb_container = user_data;

	RET_IF(NULL == resource);
	RET_IF(NULL == response);
	RETM_IF(IOTCON_ERROR_NONE != err, "iotcon_remote_resource_get Fail(%d)", err);

	ret = icl_representation_compare(resource->cached_repr, response->repr);
	if (IC_EQUAL != ret) { /* updated */
		if (resource->cached_repr)
			iotcon_representation_destroy(resource->cached_repr);
		resource->cached_repr = response->repr;
		response->repr = NULL;
		cb_container->cb(resource, resource->cached_repr, cb_container->user_data);
	}
}

static gboolean _icl_ioty_caching_idle_cb(gpointer p)
{
	int ret;
	icl_caching_container_s *cb_container = p;

	RETV_IF(NULL == cb_container, G_SOURCE_REMOVE);

	/* get */
	ret = icl_ioty_remote_resource_get(cb_container->resource, NULL,
			_icl_ioty_caching_get_cb, cb_container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_remote_resource_get() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}

	/* get timeout */
	if (cb_container->timeout)
		g_source_remove(cb_container->timeout);
	cb_container->timeout = g_timeout_add_seconds(icl_remote_resource_time_interval,
			_icl_ioty_caching_idle_cb, cb_container);

	return G_SOURCE_REMOVE;
}

static void _icl_ioty_caching_observe_cb(iotcon_remote_resource_h resource,
		iotcon_error_e err, int sequence_number, iotcon_response_h response, void *user_data)
{
	FN_CALL;
	icl_caching_container_s *cb_container = user_data;

	RET_IF(NULL == resource);
	RET_IF(NULL == cb_container);

	g_idle_add(_icl_ioty_caching_idle_cb, cb_container);
}

static void _icl_ioty_remote_resource_caching_table_insert(
		iotcon_remote_resource_h resource, icl_caching_container_s *cb_container)
{
	RET_IF(NULL == resource);
	RET_IF(NULL == cb_container);

	if (NULL == icl_caching_table) {
		icl_caching_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
				icl_destroy_caching_container);
	}
	g_hash_table_insert(icl_caching_table, resource, cb_container);
}

static int _icl_ioty_remote_resource_caching_table_remove(
		iotcon_remote_resource_h resource)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == icl_caching_table)
		return IOTCON_ERROR_NO_DATA;

	if (NULL == g_hash_table_lookup(icl_caching_table, resource))
		return IOTCON_ERROR_NO_DATA;

	g_hash_table_remove(icl_caching_table, resource);

	return IOTCON_ERROR_NONE;
}



int icl_ioty_remote_resource_start_caching(iotcon_remote_resource_h resource,
		iotcon_remote_resource_cached_representation_changed_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	OCDoHandle handle = NULL;
	icl_caching_container_s *cb_container;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	cb_container = calloc(1, sizeof(icl_caching_container_s));
	if (NULL == cb_container) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_container->resource = resource;
	icl_remote_resource_ref(cb_container->resource);
	cb_container->cb = cb;
	cb_container->user_data = user_data;

	ret = _icl_ioty_remote_resource_observe(resource, IOTCON_OBSERVE_IGNORE_OUT_OF_ORDER,
			NULL, _icl_ioty_caching_observe_cb, cb_container, &handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_remote_resource_observe() Fail(%d)", ret);
		icl_destroy_caching_container(cb_container);
		return ret;
	}
	cb_container->observe_handle = IC_POINTER_TO_INT64(handle);

	_icl_ioty_remote_resource_caching_table_insert(resource, cb_container);

	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_stop_caching(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = _icl_ioty_remote_resource_caching_table_remove(resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_remote_resource_caching_table_remove() Fail(%d)", ret);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


int icl_ioty_start_presence(unsigned int time_to_live)
{
	int ret;
	RETV_IF(IC_PRESENCE_TTL_SECONDS_MAX < time_to_live, IOTCON_ERROR_INVALID_PARAMETER);

	icl_ioty_csdk_lock();
	ret = OCStartPresence(time_to_live);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCStartPresence() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_stop_presence()
{
	int ret;

	icl_ioty_csdk_lock();
	ret = OCStopPresence();
	icl_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCStopPresence() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_resource_bind_type(OCResourceHandle handle, const char *resource_type)
{
	OCStackResult ret;

	icl_ioty_csdk_lock();
	ret = OCBindResourceTypeToResource(handle, resource_type);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCBindResourceTypeToResource() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_resource_bind_interface(OCResourceHandle handle, const char *iface)
{
	OCStackResult result;

	icl_ioty_csdk_lock();
	result = OCBindResourceInterfaceToResource(handle, iface);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCBindResourceInterfaceToResource() Fail(%d)", result);
		return ic_ioty_parse_oic_error(result);
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		iotcon_resource_interfaces_h ifaces,
		int properties,
		iotcon_request_handler_cb cb,
		void *user_data,
		iotcon_resource_h *resource_handle)
{
	FN_CALL;
	GList *c;
	OCResourceHandle handle;
	int ret, i;
	iotcon_resource_h resource;

	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(ICL_URI_PATH_LENGTH_MAX < strlen(uri_path),
			IOTCON_ERROR_INVALID_PARAMETER, "Invalid uri_path(%s)", uri_path);
	RETV_IF(NULL == res_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_handle, IOTCON_ERROR_INVALID_PARAMETER);

	resource = calloc(1, sizeof(struct icl_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		OCDeleteResource(handle);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	resource->uri_path = strdup(uri_path);
	resource->properties = properties;
	resource->types = icl_resource_types_ref(res_types);
	resource->ifaces = icl_resource_interfaces_ref(ifaces);

	resource->cb = cb;
	resource->user_data = user_data;

	properties = ic_ioty_convert_properties(properties);

	icl_ioty_csdk_lock();
	ret = OCCreateResource(&handle, res_types->type_list->data, ifaces->iface_list->data,
			uri_path, icl_ioty_ocprocess_request_cb, resource, properties);
	icl_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCCreateResource() Fail(%d)", ret);
		iotcon_resource_destroy(resource);
		return IOTCON_ERROR_IOTIVITY;
	}
	resource->handle = IC_POINTER_TO_INT64(handle);

	for (c = res_types->type_list, i = 0; c; c = c->next, i++) {
		if (0 == i) /* skip a first resource type */
			continue;
		_icl_ioty_resource_bind_type(handle, c->data);
	}

	for (c = ifaces->iface_list, i = 0; c; c = c->next, i++) {
		if (0 == i) /* skip a first resource iface */
			continue;
		_icl_ioty_resource_bind_interface(handle, c->data);
	}

	*resource_handle = resource;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_resource_bind_type(iotcon_resource_h resource,
		const char *resource_type)
{
	FN_CALL;
	int ret;
	OCResourceHandle handle;
	iotcon_resource_types_h resource_types;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_type, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_resource_types_clone(resource->types, &resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_clone() Fail(%d)", ret);
		return ret;
	}

	ret = iotcon_resource_types_add(resource_types, resource_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_add() Fail(%d)", ret);
		iotcon_resource_types_destroy(resource_types);
		return ret;
	}

	handle = IC_INT64_TO_POINTER(resource->handle);
	ret = _icl_ioty_resource_bind_type(handle, resource_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_resource_bind_type Fail(%d)", ret);
		iotcon_resource_types_destroy(resource_types);
		return ret;
	}

	iotcon_resource_types_destroy(resource->types);
	resource->types = resource_types;

	return ret;
}

int icl_ioty_resource_bind_interface(iotcon_resource_h resource,
		const char *iface)
{
	FN_CALL;
	int ret;
	OCResourceHandle handle;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_resource_interfaces_add(resource->ifaces, iface);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_interfaces_add() Fail(%d)", ret);
		return ret;
	}

	handle = IC_INT64_TO_POINTER(resource->handle);
	ret = _icl_ioty_resource_bind_interface(handle, iface);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_ioty_resource_bind_interface() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_resource_bind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child)
{
	FN_CALL;
	int i, ret;
	OCResourceHandle handle_parent, handle_child;

	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(parent == child, IOTCON_ERROR_INVALID_PARAMETER);

	for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
		if (child == parent->children[i]) {
			ERR("Child resource was already bound to parent resource.");
			return IOTCON_ERROR_ALREADY;
		}
	}

	handle_parent = IC_INT64_TO_POINTER(parent->handle);
	handle_child = IC_INT64_TO_POINTER(child->handle);

	for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
		if (NULL == parent->children[i]) {
			icl_ioty_csdk_lock();
			ret = OCBindResource(handle_parent, handle_child);
			icl_ioty_csdk_unlock();

			if (OC_STACK_OK != ret) {
				ERR("OCBindResource() Fail(%d)", ret);
				return ic_ioty_parse_oic_error(ret);
			}

			parent->children[i] = child;
			return IOTCON_ERROR_NONE;
		}
	}

	ERR("There is no slot to bind a child resource");
	return IOTCON_ERROR_OUT_OF_MEMORY;
}

int icl_ioty_resource_unbind_child_resource(iotcon_resource_h parent,
		iotcon_resource_h child)
{
	FN_CALL;
	int i, ret;
	OCResourceHandle handle_parent, handle_child;

	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	handle_parent = IC_INT64_TO_POINTER(parent->handle);
	handle_child = IC_INT64_TO_POINTER(child->handle);

	icl_ioty_csdk_lock();
	ret = OCUnBindResource(handle_parent, handle_child);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCUnBindResource() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	for (i = 0; i < ICL_CONTAINED_RESOURCES_MAX; i++) {
		if (child == parent->children[i])
			parent->children[i] = NULL;
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_resource_notify(iotcon_resource_h resource, iotcon_representation_h repr,
		iotcon_observers_h observers, iotcon_qos_e qos)
{
	int ret, obs_length = 0;
	GList *c;
	OCRepPayload *payload = NULL;
	iotcon_observers_h observers_ori;
	OCObservationId obs_ids[IC_OBSERVE_ID_MAX_LEN];
	OCQualityOfService oc_qos;
	OCResourceHandle handle;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	observers_ori = (observers) ? observers : resource->observers;
	if (NULL == observers_ori) {
		DBG("observers_ori is NULL");
		return IOTCON_ERROR_NONE;
	}
	for (c = observers_ori->observers_list; c; c = c->next) {
		int observe_id = GPOINTER_TO_INT(c->data);
		obs_ids[obs_length++] = observe_id;
	}
	if (0 == obs_length) {
		DBG("No observers for lib");
		return IOTCON_ERROR_NONE;
	}

	ret = icl_ioty_convert_representation(repr, (OCPayload **)&payload);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_convert_representation() Fail(%d)", ret);
		return ret;
	}

	oc_qos = ic_ioty_convert_qos(qos);

	handle = IC_INT64_TO_POINTER(resource->handle);

	icl_ioty_csdk_lock();
	if (payload)
		ret = OCNotifyListOfObservers(handle, obs_ids, obs_length, payload, oc_qos);
	else
		ret = OCNotifyAllObservers(handle, oc_qos);
	icl_ioty_csdk_unlock();

	if (OC_STACK_NO_OBSERVERS == ret) {
		WARN("No Observers. Stop Notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ret) {
		ERR("OCNotifyListOfObservers() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_resource_destroy(iotcon_resource_h resource)
{
	FN_CALL;
	int ret;
	OCResourceHandle handle;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == resource->handle) { /* iotcon dbus disconnected */
		WARN("Invalid Resource handle");
		return IOTCON_ERROR_NONE;
	}

	handle = IC_INT64_TO_POINTER(resource->handle);

	icl_ioty_csdk_lock();
	ret = OCDeleteResource(handle);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCDeleteResource() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}
	resource->handle = 0;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_lite_resource_create(const char *uri_path,
		iotcon_resource_types_h res_types,
		int properties,
		iotcon_state_h state,
		iotcon_lite_resource_post_request_cb cb,
		void *user_data,
		iotcon_lite_resource_h *resource_handle)
{

	FN_CALL;
	int ret, i;
	GList *c;
	OCResourceHandle handle;
	iotcon_lite_resource_h resource;
	const char *res_iface = NULL;

	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(ICL_URI_PATH_LENGTH_MAX < strlen(uri_path),
			IOTCON_ERROR_INVALID_PARAMETER, "Invalid uri_path(%s)", uri_path);
	RETV_IF(NULL == res_types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_handle, IOTCON_ERROR_INVALID_PARAMETER);

	resource = calloc(1, sizeof(struct icl_lite_resource));
	if (NULL == resource) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	resource->uri_path = strdup(uri_path);
	resource->properties = properties;
	resource->state = state;
	icl_state_ref(resource->state);
	resource->cb = cb;
	resource->cb_data = user_data;

	res_iface = IOTCON_INTERFACE_DEFAULT;

	properties = ic_ioty_convert_properties(properties);

	icl_ioty_csdk_lock();
	ret = OCCreateResource(&handle, res_types->type_list->data, res_iface, uri_path,
			icl_ioty_ocprocess_lite_request_cb, resource, properties);
	icl_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCCreateResource() Fail(%d)", ret);
		icl_ioty_lite_resource_destroy(resource);
		return IOTCON_ERROR_IOTIVITY;
	}
	resource->handle = IC_POINTER_TO_INT64(handle);

	for (c = res_types->type_list, i = 0; c; c = c->next, i++) {
		if (0 == i) /* skip a first resource type */
			continue;
		_icl_ioty_resource_bind_type(handle, c->data);
	}

	*resource_handle = resource;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_lite_resource_destroy(iotcon_lite_resource_h resource)
{
	FN_CALL;
	int ret;
	OCResourceHandle handle;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == resource->handle) { /* iotcon dbus disconnected */
		WARN("Invalid Resource handle");
		return IOTCON_ERROR_NONE;
	}

	handle = IC_INT64_TO_POINTER(resource->handle);

	icl_ioty_csdk_lock();
	ret = OCDeleteResource(handle);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCDeleteResource() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}
	resource->handle = 0;
	free(resource->uri_path);
	free(resource);

	return IOTCON_ERROR_NONE;
}

int icl_ioty_lite_resource_notify(iotcon_lite_resource_h resource)
{
	FN_CALL;
	int ret;
	OCResourceHandle handle;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	handle = IC_INT64_TO_POINTER(resource->handle);

	icl_ioty_csdk_lock();
	ret = OCNotifyAllObservers(handle, IOTCON_QOS_HIGH);
	icl_ioty_csdk_unlock();

	if (OC_STACK_NO_OBSERVERS == ret) {
		WARN("No Observers. Stop Notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ret) {
		ERR("OCNotifyListOfObservers() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

int icl_ioty_lite_resource_update_state(iotcon_lite_resource_h resource,
		iotcon_state_h state)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (state)
		state = icl_state_ref(state);

	if (resource->state)
		iotcon_state_destroy(resource->state);

	resource->state = state;

	ret = icl_ioty_lite_resource_notify(resource);
	if (IOTCON_ERROR_NONE != ret)
		WARN("icl_ioty_lite_resource_notify() Fail");

	return IOTCON_ERROR_NONE;

}

int icl_ioty_response_send(iotcon_response_h response_handle)
{
	FN_CALL;
	int ret;
	OCEntityHandlerResponse response = {0};

	RETV_IF(NULL == response_handle, IOTCON_ERROR_INVALID_PARAMETER);

	response.requestHandle = IC_INT64_TO_POINTER(response_handle->oic_request_h);
	response.resourceHandle = IC_INT64_TO_POINTER(response_handle->oic_resource_h);
	response.ehResult = ic_ioty_convert_response_result(response_handle->result);

	if (response_handle->header_options && response_handle->header_options->hash) {
		icl_ioty_convert_header_options(response_handle->header_options,
				response.sendVendorSpecificHeaderOptions,
				sizeof(response.sendVendorSpecificHeaderOptions)
				/ sizeof(response.sendVendorSpecificHeaderOptions[0]));

		response.numSendVendorSpecificHeaderOptions =
				g_hash_table_size(response_handle->header_options->hash);
	}


	icl_ioty_convert_representation(response_handle->repr, &(response.payload));

	/* related to block transfer */
	response.persistentBufferFlag = 0;

	icl_ioty_csdk_lock();
	ret = OCDoResponse(&response);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCDoResponse() Fail(%d)", ret);
		return ic_ioty_parse_oic_error(ret);
	}

	return IOTCON_ERROR_NONE;
}

static void _icl_ioty_caching_table_update(gpointer key, gpointer value, gpointer user_data)
{
	icl_caching_container_s *cb_container = value;
	g_source_remove(cb_container->timeout);
	cb_container->timeout = g_timeout_add_seconds(icl_remote_resource_time_interval,
			_icl_ioty_caching_idle_cb, cb_container);
}

static void _icl_ioty_monitoring_table_update(gpointer key, gpointer value, gpointer user_data)
{
	icl_monitoring_container_s *cb_container = value;
	g_source_remove(cb_container->timeout);
	cb_container->timeout = g_timeout_add_seconds(icl_remote_resource_time_interval,
			_icl_ioty_monitoring_idle_cb, cb_container);
}

static void _icl_ioty_remote_resource_update_time_interval()
{
	if (icl_caching_table)
		g_hash_table_foreach(icl_caching_table, _icl_ioty_caching_table_update, NULL);
	if (icl_monitoring_table)
		g_hash_table_foreach(icl_monitoring_table, _icl_ioty_monitoring_table_update, NULL);
}

int icl_ioty_remote_resource_set_time_interval(int time_interval)
{
	icl_remote_resource_time_interval = time_interval;
	_icl_ioty_remote_resource_update_time_interval();
	return IOTCON_ERROR_NONE;
}

int icl_ioty_remote_resource_get_time_interval(int *time_interval)
{
	RETV_IF(NULL == time_interval, IOTCON_ERROR_INVALID_PARAMETER);
	*time_interval = icl_remote_resource_time_interval;
	return IOTCON_ERROR_NONE;
}
