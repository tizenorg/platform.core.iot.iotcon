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
#include "icl-ioty.h"
#include "icl-repr.h"
#include "icl-options.h"
#include "icl-request.h"
#include "icl-response.h"

API iotcon_response_h iotcon_response_new(iotcon_request_h request_h)
{
	FN_CALL;

	RETV_IF(NULL == request_h, NULL);

	iotcon_response_h resp = calloc(1, sizeof(struct ic_resource_response));
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
	if (resp->new_uri)
		free(resp->new_uri);
	if (resp->header_options)
		iotcon_options_free(resp->header_options);
	free(resp);
}


API int iotcon_response_set(iotcon_response_h resp, iotcon_response_property_e prop, ...)
{
	int value;
	va_list args;
	char *new_resource_uri = NULL;
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
	case IOTCON_RESPONSE_RESOURCE_URI:
		new_resource_uri = va_arg(args, char*);
		if (resp->new_uri)
			free(resp->new_uri);

		if (new_resource_uri) {
			resp->new_uri = strdup(new_resource_uri);
			if (NULL == resp->new_uri) {
				ERR("strdup() Fail(%d)", errno);
				va_end(args);
				return IOTCON_ERROR_OUT_OF_MEMORY;
			}
		} else {
			resp->new_uri = NULL;
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


API int iotcon_response_send(iotcon_response_h resp)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resp, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resp->repr, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_dbus_send_response(resp);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_send_response() Fail(%d)", ret);

	return ret;
}

