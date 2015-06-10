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
#include <stdbool.h>
#include <glib.h>
#include <iotcon.h>
#include "test-log.h"

static void _room_request_handler(iotcon_request_h request, void *user_data);

static void _send_response(iotcon_response_h response, iotcon_repr_h repr,
		iotcon_interface_e interface)
{
	iotcon_response_set(response, IOTCON_RESPONSE_REPRESENTATION, repr);
	iotcon_response_set(response, IOTCON_RESPONSE_INTERFACE, interface);
	iotcon_response_set(response, IOTCON_RESPONSE_RESULT, IOTCON_RESPONSE_RESULT_OK);

	/* send Representation to the client */
	iotcon_response_send(response);
}

static void _light_request_handler_get(iotcon_response_h response)
{
	iotcon_repr_h resp_repr;

	INFO("GET request - Light");

	/* create a light Representation */
	resp_repr = iotcon_repr_new();

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_repr_free(resp_repr);
}

static int _query_foreach_cb(const char *key, const char *value, void *user_data)
{
	char **interface_str = user_data;

	if (!strcmp("if", key)) {
		*interface_str = (char*)value;
	}
	return IOTCON_FUNC_CONTINUE;
}

static void _room_request_handler_get(iotcon_request_h request,
		iotcon_response_h response)
{
	int ret;
	iotcon_repr_h room_repr;
	iotcon_repr_h light_repr;
	iotcon_repr_h switch_repr;
	iotcon_list_h temperature_list;

	iotcon_query_h query;
	char *query_str = NULL;

	iotcon_interface_e interface;

	INFO("GET request - Room");

	/* create a room Representation */
	room_repr = iotcon_repr_new();
	iotcon_repr_set_uri(room_repr, "/a/room");
	iotcon_repr_set_str(room_repr, "name", "Michael's Room");

	/* set null */
	iotcon_repr_set_null(room_repr, "null value");

	temperature_list = iotcon_list_new(IOTCON_TYPE_INT);
	iotcon_list_insert_int(temperature_list, 22, -1);
	iotcon_list_insert_int(temperature_list, 23, -1);
	iotcon_list_insert_int(temperature_list, 24, -1);
	iotcon_list_insert_int(temperature_list, 25, -1);
	iotcon_list_insert_int(temperature_list, 26, -1);
	iotcon_repr_set_list(room_repr, "today_temp", temperature_list);
	iotcon_list_free(temperature_list);

	/* create a light Representation */
	light_repr = iotcon_repr_new();
	iotcon_repr_set_uri(light_repr, "/a/light");
	iotcon_repr_set_int(light_repr, "brightness", 50);

	/* create a switch Representation */
	switch_repr = iotcon_repr_new();
	iotcon_repr_set_uri(switch_repr, "/a/switch");
	iotcon_repr_set_bool(switch_repr, "switch", false);

	iotcon_repr_append_child(room_repr, light_repr);
	iotcon_repr_append_child(room_repr, switch_repr);
	iotcon_repr_free(light_repr);
	iotcon_repr_free(switch_repr);

	ret = iotcon_request_get_query(request, &query);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_query() Fail(%d)", ret);
		iotcon_repr_free(room_repr);
		return;
	}
	if (query)
		iotcon_query_foreach(query, _query_foreach_cb, &query_str);

	if (query_str && !strcmp("oc.mi.b", query_str)) {
		DBG("operation for BATCH interface");
		interface = IOTCON_INTERFACE_BATCH;
	}
	else {
		DBG("operation for DEFAULT interface");
		interface = IOTCON_INTERFACE_DEFAULT;
	}

	_send_response(response, room_repr, interface);
	iotcon_repr_free(room_repr);

	FN_END;
}

static void _request_handler_put(iotcon_request_h request, iotcon_response_h response)
{
	iotcon_repr_h resp_repr = iotcon_repr_new();

	INFO("PUT request");

	/* do PUT operation */

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_repr_free(resp_repr);
}

static void _request_handler_post(iotcon_response_h response)
{
	iotcon_repr_h resp_repr = iotcon_repr_new();

	INFO("POST request");

	/* do POST operation */

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_repr_free(resp_repr);

}

static void _request_handler_delete(iotcon_response_h response)
{
	iotcon_repr_h resp_repr = iotcon_repr_new();

	INFO("DELETE request");

	/* do DELETE operation */

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_repr_free(resp_repr);

}

static void _light_request_handler(iotcon_request_h request, void *user_data)
{
	int ret;
	int types;
	iotcon_response_h response;
	FN_CALL;

	RET_IF(NULL == request);

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_types() Fail(%d)", ret);
		return;
	}

	response = iotcon_response_new(request);
	if (NULL == response) {
		ERR("iotcon_response_new() Fail");
		return;
		}

	if (IOTCON_REQUEST_GET & types)
		_light_request_handler_get(response);

	else if (IOTCON_REQUEST_PUT & types)
		_request_handler_put(request, response);

	else if (IOTCON_REQUEST_POST & types)
		_request_handler_post(response);

	else if (IOTCON_REQUEST_DELETE & types)
		_request_handler_delete(response);

	iotcon_response_free(response);
}

static void _room_request_handler(iotcon_request_h request, void *user_data)
{
	int ret;
	int types;
	iotcon_response_h response;
	FN_CALL;

	RET_IF(NULL == request);

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_types() Fail(%d)", ret);
		return;
	}

	response = iotcon_response_new(request);
	if (NULL == response) {
		ERR("iotcon_response_new() Fail");
		return;
		}

	if (IOTCON_REQUEST_GET & types)
		_room_request_handler_get(request, response);

	else if (IOTCON_REQUEST_PUT & types)
		_request_handler_put(request, response);

	else if (IOTCON_REQUEST_POST & types)
		_request_handler_post(response);

	else if (IOTCON_REQUEST_DELETE & types)
		_request_handler_delete(response);

	iotcon_response_free(response);
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	iotcon_resource_types_h room_rtypes = NULL;
	iotcon_resource_types_h light_rtypes = NULL;
	iotcon_error_e iotcon_error = IOTCON_ERROR_NONE;

	loop = g_main_loop_new(NULL, FALSE);

	/* initialize address and port */
	iotcon_initialize(IOTCON_ALL_INTERFACES, IOTCON_RANDOM_PORT);

	/* register room resource */
	room_rtypes = iotcon_resource_types_new();
	iotcon_resource_types_insert(room_rtypes, "core.room");
	iotcon_resource_h room_handle = iotcon_register_resource("/a/room", room_rtypes,
			(IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_BATCH),
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), _room_request_handler,
			NULL);
	if (NULL == room_handle) {
		ERR("iotcon_register_resource() Fail");
		return -1;
	}

	/* register room resource */
	light_rtypes = iotcon_resource_types_new();
	if (NULL == light_rtypes) {
		ERR("iotcon_resource_types_new() Fail");
		return -1;
	}

	iotcon_resource_types_insert(light_rtypes, "core.light");
	iotcon_resource_h light_handle = iotcon_register_resource("/a/light", light_rtypes,
			(IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_BATCH),
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), _light_request_handler,
			NULL);
	if (NULL == light_handle) {
		ERR("iotcon_register_resource() Fail");
		return -1;
	}

	iotcon_error = iotcon_bind_resource(room_handle, light_handle);
	if (IOTCON_ERROR_NONE != iotcon_error) {
		ERR("iotcon_bind_resource() Fail");
		return -1;
	}

	iotcon_resource_types_free(light_rtypes);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	iotcon_unregister_resource(room_handle);

	return 0;
}
