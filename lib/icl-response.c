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
#include <stdlib.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-resource.h"
#include "icl-repr.h"
#include "icl-options.h"
#include "icl-request.h"
#include "icl-response.h"

API iotcon_response_h iotcon_response_new(iotcon_request_h request_h)
{
	FN_CALL;

	RETV_IF(NULL == request_h, NULL);

	iotcon_response_h resp = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == resp) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	resp->request_handle = request_h->request_handle;
	resp->resource_handle = request_h->resource_handle;
	resp->error_code = 200;

	return resp;
}


API void iotcon_response_free(iotcon_response_h resp)
{
	FN_CALL;

	RET_IF(NULL == resp);

	if (resp->repr)
		iotcon_repr_free(resp->repr);
	if (resp->new_uri_path)
		free(resp->new_uri_path);
	if (resp->header_options)
		iotcon_options_free(resp->header_options);
	free(resp);
}


API int iotcon_response_set(iotcon_response_h resp, iotcon_response_property_e prop, ...)
{
	int value;
	va_list args;
	char *new_uri_path = NULL;
	iotcon_options_h options = NULL;

	va_start(args, prop);

	switch (prop) {
	case IOTCON_RESPONSE_INTERFACE:
		resp->iface = va_arg(args, int);
		break;
	case IOTCON_RESPONSE_REPRESENTATION:
		resp->repr = va_arg(args, iotcon_repr_h);
		icl_repr_inc_ref_count(resp->repr);
		break;
	case IOTCON_RESPONSE_RESULT:
		value = va_arg(args, int);
		if (value < IOTCON_RESPONSE_RESULT_OK || IOTCON_RESPONSE_RESULT_MAX <= value) {
			ERR("Invalid value(%d)", value);
			va_end(args);
			return IOTCON_ERROR_INVALID_PARAMETER;
		}
		resp->result = value;
		break;
	case IOTCON_RESPONSE_NEW_URI_PATH:
		new_uri_path = va_arg(args, char*);
		if (resp->new_uri_path)
			free(resp->new_uri_path);

		if (new_uri_path) {
			resp->new_uri_path = strdup(new_uri_path);
			if (NULL == resp->new_uri_path) {
				ERR("strdup() Fail(%d)", errno);
				va_end(args);
				return IOTCON_ERROR_OUT_OF_MEMORY;
			}
		} else {
			resp->new_uri_path = NULL;
		}
		break;
	case IOTCON_RESPONSE_HEADER_OPTIONS:
		options = va_arg(args, iotcon_options_h);
		if (resp->header_options)
			iotcon_options_free(resp->header_options);

		if (options)
			resp->header_options = icl_options_ref(options);
		else
			resp->header_options = NULL;
		break;
	default:
		ERR("Invalid Response Property(%d)", prop);
		break;
	}

	va_end(args);

	return IOTCON_ERROR_NONE;
}


static bool _icl_response_repr_child_fn(iotcon_repr_h child, void *user_data)
{
	int iface = GPOINTER_TO_INT(user_data);

	if (IOTCON_INTERFACE_DEFAULT == iface)
		child->visibility = ICL_VISIBILITY_PROP;
	else if (IOTCON_INTERFACE_LINK == iface)
		child->visibility = ICL_VISIBILITY_PROP;
	else if (IOTCON_INTERFACE_BATCH == iface)
		child->visibility = ICL_VISIBILITY_REPR;
	else
		child->visibility = ICL_VISIBILITY_PROP;

	return IOTCON_FUNC_CONTINUE;
}


static int _icl_response_check_repr_visibility(iotcon_response_h resp)
{
	int ret;

	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resp->repr, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_repr_h first = resp->repr;

	DBG("interface type of response : %d", resp->iface);
	if (IOTCON_INTERFACE_NONE == resp->iface)
		first->visibility = ICL_VISIBILITY_REPR;
	else if (IOTCON_INTERFACE_DEFAULT == resp->iface)
		first->visibility = ICL_VISIBILITY_REPR;
	else
		first->visibility = ICL_VISIBILITY_NONE;

	ret = iotcon_repr_foreach_children(first, _icl_response_repr_child_fn,
			GINT_TO_POINTER(resp->iface));
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_repr_foreach_children() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_response_send(iotcon_response_h resp)
{
	FN_CALL;
	int ret;
	GError *error = NULL;
	GVariant *arg_response;

	RETV_IF(NULL == icl_dbus_get_object(), IOTCON_ERROR_DBUS);
	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resp->repr, IOTCON_ERROR_INVALID_PARAMETER);

	ret = _icl_response_check_repr_visibility(resp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_response_check_repr_visibility() Fail(%d)", ret);
		return ret;
	}

	arg_response = icl_dbus_response_to_gvariant(resp);
	ic_dbus_call_send_response_sync(icl_dbus_get_object(), arg_response, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_send_response_sync() Fail(%s)", error->message);
		g_error_free(error);
		g_variant_unref(arg_response);
		return IOTCON_ERROR_DBUS;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}
