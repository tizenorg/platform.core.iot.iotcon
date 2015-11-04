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
#include "icl-representation.h"
#include "icl-options.h"
#include "icl-request.h"
#include "icl-response.h"

/* the last index of iotcon_response_result_e */
#define ICL_RESPONSE_RESULT_MAX (IOTCON_RESPONSE_RESULT_FORBIDDEN + 1)

API int iotcon_response_create(iotcon_request_h request,
		iotcon_response_h *response)
{
	FN_CALL;

	RETV_IF(NULL == request, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == response, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_response_h resp = calloc(1, sizeof(struct icl_resource_response));
	if (NULL == resp) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	resp->oic_request_h = request->oic_request_h;
	resp->oic_resource_h = request->oic_resource_h;

	*response = resp;

	return IOTCON_ERROR_NONE;
}


API void iotcon_response_destroy(iotcon_response_h resp)
{
	RET_IF(NULL == resp);

	if (resp->repr)
		iotcon_representation_destroy(resp->repr);
	if (resp->header_options)
		iotcon_options_destroy(resp->header_options);
	free(resp);
}

API int iotcon_response_get_options(iotcon_response_h resp,
		iotcon_options_h *options)
{
	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resp->header_options, IOTCON_ERROR_NO_DATA);

	*options = resp->header_options;

	return IOTCON_ERROR_NONE;
}

API int iotcon_response_get_representation(iotcon_response_h resp,
		iotcon_representation_h *repr)
{
	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resp->repr, IOTCON_ERROR_NO_DATA);

	*repr = resp->repr;

	return IOTCON_ERROR_NONE;

}

API int iotcon_response_get_result(iotcon_response_h resp, int *result)
{
	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);

	*result = resp->result;

	return IOTCON_ERROR_NONE;
}

API int iotcon_response_set_result(iotcon_response_h resp,
		iotcon_response_result_e result)
{
	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);

	if (result < IOTCON_RESPONSE_RESULT_OK || ICL_RESPONSE_RESULT_MAX <= result) {
		ERR("Invalid result(%d)", result);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	resp->result = result;

	return IOTCON_ERROR_NONE;
}


API int iotcon_response_set_representation(iotcon_response_h resp,
		iotcon_interface_e iface, iotcon_representation_h repr)
{
	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	resp->repr = repr;
	resp->iface = iface;
	icl_representation_inc_ref_count(resp->repr);

	return IOTCON_ERROR_NONE;
}


API int iotcon_response_set_header_options(iotcon_response_h resp,
		iotcon_options_h options)
{
	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);

	if (resp->header_options)
		iotcon_options_destroy(resp->header_options);

	if (options)
		resp->header_options = icl_options_ref(options);
	else
		resp->header_options = NULL;

	return IOTCON_ERROR_NONE;
}

static bool _icl_response_representation_child_cb(iotcon_representation_h child,
		void *user_data)
{
	int iface = GPOINTER_TO_INT(user_data);

	switch(iface) {
	case IOTCON_INTERFACE_BATCH:
		child->visibility = ICL_VISIBILITY_REPR;
		break;
	case IOTCON_INTERFACE_NONE:
	case IOTCON_INTERFACE_DEFAULT:
	case IOTCON_INTERFACE_LINK:
	case IOTCON_INTERFACE_GROUP:
		child->visibility = ICL_VISIBILITY_PROP;
		break;
	default:
		WARN("Invalid interface type(%d)", iface);
		child->visibility = ICL_VISIBILITY_PROP;
		break;
	}

	return IOTCON_FUNC_CONTINUE;
}


static int _icl_response_check_representation_visibility(iotcon_response_h resp)
{
	int ret;

	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resp->repr, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_representation_h first = resp->repr;

	DBG("interface type of response : %d", resp->iface);

	switch(resp->iface) {
	case IOTCON_INTERFACE_NONE:
	case IOTCON_INTERFACE_DEFAULT:
	case IOTCON_INTERFACE_GROUP:
		first->visibility = ICL_VISIBILITY_REPR;
		break;
	case IOTCON_INTERFACE_LINK:
	case IOTCON_INTERFACE_BATCH:
		first->visibility = ICL_VISIBILITY_NONE;
		break;
	default:
		WARN("Invalid interface type(%d)", resp->iface);
		first->visibility = ICL_VISIBILITY_REPR;
		break;
	}

	ret = iotcon_representation_foreach_children(first,
			_icl_response_representation_child_cb, GINT_TO_POINTER(resp->iface));
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_foreach_children() Fail(%d)", ret);
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

	ret = _icl_response_check_representation_visibility(resp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_response_check_representation_visibility() Fail(%d)", ret);
		return ret;
	}

	arg_response = icl_dbus_response_to_gvariant(resp);
	ic_dbus_call_send_response_sync(icl_dbus_get_object(), arg_response, &ret, NULL,
			&error);
	if (error) {
		ERR("ic_dbus_call_send_response_sync() Fail(%s)", error->message);
		ret = icl_dbus_convert_dbus_error(error->code);
		g_error_free(error);
		g_variant_unref(arg_response);
		return ret;
	}

	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon-daemon Fail(%d)", ret);
		return icl_dbus_convert_daemon_error(ret);
	}

	return IOTCON_ERROR_NONE;
}
