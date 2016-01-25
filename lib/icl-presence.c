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
#include <errno.h>
#include <glib.h>

#include "iotcon-types.h"
#include "iotcon-internal.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource-types.h"
#include "icl-dbus.h"
#include "icl-presence.h"

#define ICL_PRESENCE_TTL_SECONDS_MAX (60 * 60 * 24) /* 60 sec/min * 60 min/hr * 24 hr/day */


API int iotcon_start_presence(unsigned int time_to_live)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(ICL_PRESENCE_TTL_SECONDS_MAX < time_to_live, IOTCON_ERROR_INVALID_PARAMETER);

	ic_dbus_call_start_presence_sync(icl_dbus_get_object(), time_to_live, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_start_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_stop_presence(void)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);

	ic_dbus_call_stop_presence_sync(icl_dbus_get_object(), &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_stop_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}


static void _icl_presence_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	FN_CALL;
	unsigned int nonce;
	char *host_address, *resource_type;
	int res, connectivity_type, trigger;
	icl_presence_s *presence = user_data;
	iotcon_presence_cb cb = presence->cb;
	icl_presence_response_s response = {0};

	g_variant_get(parameters, "(iu&sii&s)", &res, &nonce, &host_address,
			&connectivity_type, &trigger, &resource_type);

	response.resource_type = ic_utils_dbus_decode_str(resource_type);

	if (response.resource_type && presence->resource_type) {
		if (IC_STR_EQUAL != strcmp(response.resource_type, presence->resource_type))
			return;
	}

	if (res < IOTCON_ERROR_NONE && cb)
		cb(presence, icl_dbus_convert_daemon_error(res), NULL, presence->user_data);

	DBG("presence nonce : %u", nonce);

	response.host_address = host_address;
	response.connectivity_type = connectivity_type;
	response.result = res;
	response.trigger = trigger;

	if (cb)
		cb(presence, IOTCON_ERROR_NONE, &response, presence->user_data);
}


static void _icl_presence_conn_cleanup(icl_presence_s *presence)
{
	presence->sub_id = 0;

	if (presence->handle) {
		presence->handle = 0;
		return;
	}

	free(presence->resource_type);
	free(presence->host_address);
	free(presence);
}


/* The length of resource_type should be less than or equal to 61. */
API int iotcon_add_presence_cb(const char *host_address,
		iotcon_connectivity_type_e connectivity_type,
		const char *resource_type,
		iotcon_presence_cb cb,
		void *user_data,
		iotcon_presence_h *presence_handle)
{
	FN_CALL;
	int ret;
	unsigned int sub_id;
	GError *error = NULL;
	icl_presence_s *presence;
	char signal_name[IC_DBUS_SIGNAL_LENGTH] = {0};

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == presence_handle, IOTCON_ERROR_INVALID_PARAMETER);

	if (resource_type && (ICL_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type))) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if ((IOTCON_MULTICAST_ADDRESS == host_address || '\0' == host_address[0])
			&& (IOTCON_CONNECTIVITY_IPV4 != connectivity_type
				&& IOTCON_CONNECTIVITY_ALL != connectivity_type)) {
		ERR("Multicast is available only if IPV4");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	presence = calloc(1, sizeof(icl_presence_s));
	if (NULL == presence) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ic_dbus_call_subscribe_presence_sync(icl_dbus_get_object(),
			ic_utils_dbus_encode_str(host_address),
			connectivity_type,
			ic_utils_dbus_encode_str(resource_type),
			&(presence->handle),
			NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_subscribe_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		free(presence);
		return ret;
	}

	if (0 == presence->handle) {
		ERR("iotcon-daemon Fail");
		free(presence);
		return IOTCON_ERROR_IOTIVITY;
	}

	snprintf(signal_name, sizeof(signal_name), "%s_%llx", IC_DBUS_SIGNAL_PRESENCE,
			presence->handle);

	presence->cb = cb;
	presence->user_data = user_data;

	if (host_address)
		presence->host_address = strdup(host_address);
	presence->connectivity_type = connectivity_type;
	if (resource_type)
		presence->resource_type = strdup(resource_type);

	sub_id = icl_dbus_subscribe_signal(signal_name, presence,
			_icl_presence_conn_cleanup, _icl_presence_cb);
	if (0 == sub_id) {
		ERR("icl_dbus_subscribe_signal() Fail");
		free(presence->resource_type);
		free(presence->host_address);
		free(presence);
		return IOTCON_ERROR_DBUS;
	}

	presence->sub_id = sub_id;

	*presence_handle = presence;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remove_presence_cb(iotcon_presence_h presence)
{
	FN_CALL;
	int ret;
	GError *error = NULL;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == presence, IOTCON_ERROR_INVALID_PARAMETER);

	if (0 == presence->sub_id) { /* disconnected iotcon dbus */
		WARN("Invalid Presence handle");
		free(presence->resource_type);
		free(presence->host_address);
		free(presence);
		return IOTCON_ERROR_NONE;
	}

	if (NULL == icl_dbus_get_object()) {
		ERR("icl_dbus_get_object() return NULL");
		return IOTCON_ERROR_DBUS;
	}

	ic_dbus_call_unsubscribe_presence_sync(icl_dbus_get_object(), presence->handle,
			presence->host_address, &ret, NULL, &error);
	if (error) {
		ERR("ic_dbus_call_unsubscribe_presence_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}
	presence->handle = 0;

	icl_dbus_unsubscribe_signal(presence->sub_id);
	presence->sub_id = 0;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_get_host_address(iotcon_presence_h presence, char **host_address)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == presence, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);

	*host_address = presence->host_address;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_get_connectivity_type(iotcon_presence_h presence,
		iotcon_connectivity_type_e *connectivity_type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == presence, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == connectivity_type, IOTCON_ERROR_INVALID_PARAMETER);

	*connectivity_type = presence->connectivity_type;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_get_resource_type(iotcon_presence_h presence,
		char **resource_type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == presence, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_type, IOTCON_ERROR_INVALID_PARAMETER);

	*resource_type = presence->resource_type;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_response_get_host_address(iotcon_presence_response_h response,
		char **host_address)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);

	*host_address = response->host_address;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_response_get_connectivity_type(
		iotcon_presence_response_h response, iotcon_connectivity_type_e *connectivity_type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == connectivity_type, IOTCON_ERROR_INVALID_PARAMETER);

	*connectivity_type = response->connectivity_type;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_response_get_resource_type(iotcon_presence_response_h response,
		char **resource_type)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_type, IOTCON_ERROR_INVALID_PARAMETER);

	*resource_type = response->resource_type;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_response_get_result(iotcon_presence_response_h response,
		iotcon_presence_result_e *result)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == result, IOTCON_ERROR_INVALID_PARAMETER);

	*result = response->result;

	return IOTCON_ERROR_NONE;
}


API int iotcon_presence_response_get_trigger(iotcon_presence_response_h response,
		iotcon_presence_trigger_e *trigger)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == trigger, IOTCON_ERROR_INVALID_PARAMETER);

	if (IOTCON_PRESENCE_OK != response->result) {
		ERR("trigger is valid if IOTCON_PRESENCE_OK");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*trigger = response->trigger;

	return IOTCON_ERROR_NONE;
}

