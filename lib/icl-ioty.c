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

#include "iotcon.h"
#include "ic-utils.h"
#include "ic-ioty-type.h"
#include "icl.h"
#include "icl-dbus.h"
#include "icl-presence.h"
#include "icl-struct.h"
#include "icl-ioty-ocprocess.h"
#include "icl-ioty-struct.h"
#include "icl-ioty.h"

static GMutex icl_csdk_mutex;

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
	OCStackResult result;

	icl_ioty_ocprocess_stop();
	g_thread_join(thread);

	result = OCStop();
	if (OC_STACK_OK != result)
		ERR("OCStop() Fail(%d)", result);
}

GThread* icl_ioty_init()
{
	FN_CALL;
	GError *error;
	GThread *thread;

	OCStackResult result = OCInit(NULL, 0, OC_CLIENT_SERVER);
	if (OC_STACK_OK != result) {
		ERR("OCInit() Fail(%d)", result);
		return NULL;
	}
	DBG("OCInit() Success");

	thread = g_thread_try_new("packet_receive_thread", icl_ioty_ocprocess_thread,
			NULL, &error);
	if (NULL == thread) {
		ERR("g_thread_try_new() Fail(%s)", error->message);
		g_error_free(error);
		return NULL;
	}

	return thread;
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
	int len, timeout;
	char *coap_str;
	char uri[PATH_MAX] = {0};
	icl_cb_s *cb_data;
	OCDoHandle handle;
	OCStackResult result;
	OCCallbackData cbdata = {0};
	OCConnectivityType oic_conn_type;

	coap_str = is_secure ? IC_IOTY_COAPS : IC_IOTY_COAP;

	if (IC_STR_EQUAL == strcmp(IC_STR_NULL, host_address)) {
		len = snprintf(uri, sizeof(uri), "%s", OC_RSRVD_WELL_KNOWN_URI);
	} else {
		len = snprintf(uri, sizeof(uri), "%s%s%s", coap_str, host_address,
				OC_RSRVD_WELL_KNOWN_URI);
	}
	if (len <= 0 || sizeof(uri) <= len) {
		ERR("snprintf() Fail(%d)", len);
		return IOTCON_ERROR_IO_ERROR;
	}

	if (IC_STR_EQUAL != strcmp(IC_STR_NULL, resource_type))
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

	oic_conn_type = ic_ioty_conn_type_to_oic_conn_type(connectivity_type);

	icl_ioty_csdk_lock();
	/* TODO : QoS is come from lib. */
	result = OCDoResource(&handle, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();
	cb_data->handle = handle;

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		_icl_ioty_free_cb_data(cb_data);
		return ic_ioty_convert_error(result);
	}
	timeout = icl_dbus_get_timeout();
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

	if (IC_STR_EQUAL == strcmp(IC_STR_NULL, host_address))
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

	oic_conn_type = ic_ioty_conn_type_to_oic_conn_type(connectivity_type);

	icl_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	result = OCDoResource(&handle, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();
	cb_data->handle = handle;

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		_icl_ioty_free_cb_data(cb_data);
		return ic_ioty_convert_error(result);
	}

	timeout = icl_dbus_get_timeout();
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

	if (IC_STR_EQUAL == strcmp(IC_STR_NULL, host_address))
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

	oic_conn_type = ic_ioty_conn_type_to_oic_conn_type(connectivity_type);

	icl_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	result = OCDoResource(&handle, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();
	cb_data->handle = handle;

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		_icl_ioty_free_cb_data(cb_data);
		return ic_ioty_convert_error(result);
	}

	timeout = icl_dbus_get_timeout();
	cb_data->timeout = g_timeout_add_seconds(timeout, _icl_ioty_timeout, cb_data);

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

	if (IC_STR_EQUAL == strcmp(IC_STR_NULL, host_address) || '\0' == host_address[0])
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

	oic_conn_type = ic_ioty_conn_type_to_oic_conn_type(connectivity_type);

	icl_ioty_csdk_lock();
	result = OCDoResource(&handle, OC_REST_PRESENCE, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		icl_free_presence(presence);
		return IOTCON_ERROR_IOTIVITY;
	}

	*presence_handle = presence;

	return IOTCON_ERROR_NONE;
}

