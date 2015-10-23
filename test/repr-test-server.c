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
#include "test.h"

static int _send_response(iotcon_response_h response, iotcon_representation_h repr,
		iotcon_interface_e iface)
{
	int ret;

	ret = iotcon_response_set_representation(response, repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_set_representation() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_response_set_interface(response, iface);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_set_interface() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_response_set_result(response, IOTCON_RESPONSE_RESULT_OK);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_set_result() Fail(%d)", ret);
		return -1;
	}

	/* send Representation to the client */
	ret = iotcon_response_send(response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_send() Fail(%d)", ret);
		return -1;
	}

	return 0;
}

static void _light_request_handler_get(iotcon_response_h response)
{
	int ret;
	iotcon_representation_h resp_repr;

	INFO("GET request - Light");

	/* create a light Representation */
	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	ret = _send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(resp_repr);
}

static void _room_request_handler_get(iotcon_request_h request,
		iotcon_response_h response)
{
	int ret;
	iotcon_representation_h room_repr, light_repr, switch_repr;
	iotcon_state_h room_state, light_state, switch_state;
	iotcon_list_h temperature_list;

	iotcon_query_h query;
	char *query_str = NULL;

	iotcon_interface_e iface;

	INFO("GET request - Room");

	/* create a room Representation */
	ret = iotcon_representation_create(&room_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	/* create a room state */
	ret = iotcon_state_create(&room_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_representation_set_uri_path(room_repr, "/a/room");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_uri_path() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_state_set_str(room_state, "name", "Michael's Room");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_str() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	/* set null */
	ret = iotcon_state_set_null(room_state, "null value");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_null() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_list_create(IOTCON_TYPE_INT, &temperature_list);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_create() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_list_add_int(temperature_list, 22, -1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_add_int() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_list_add_int(temperature_list, 23, -1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_add_int() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_list_add_int(temperature_list, 24, -1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_add_int() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_list_add_int(temperature_list, 25, -1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_add_int() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_list_add_int(temperature_list, 26, -1);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_add_int() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_state_set_list(room_state, "today_temp", temperature_list);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_list() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_list_destroy(temperature_list);

	/* Set a room state into room Representation */
	ret = iotcon_representation_set_state(room_repr, room_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_state_destroy(room_state);

	/* create a light Representation */
	ret = iotcon_representation_create(&light_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_representation_set_uri_path(light_repr, "/a/light");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_uri_path() Fail(%d)", ret);
		iotcon_representation_destroy(light_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	/* create a light state */
	ret = iotcon_state_create(&light_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_state_destroy(light_state);
		iotcon_representation_destroy(light_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_state_set_int(light_state, "brightness", 50);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_int() Fail(%d)", ret);
		iotcon_state_destroy(light_state);
		iotcon_representation_destroy(light_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	/* Set a light state into light Representation */
	ret = iotcon_representation_set_state(light_repr, light_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(light_state);
		iotcon_representation_destroy(light_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_state_destroy(light_state);

	ret = iotcon_representation_append_child(room_repr, light_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_append_child() Fail(%d)", ret);
		iotcon_representation_destroy(light_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_representation_destroy(light_repr);

	/* create a switch Representation */
	ret = iotcon_representation_create(&switch_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_representation_set_uri_path(switch_repr, "/a/switch");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_uri_path() Fail(%d)", ret);
		iotcon_representation_destroy(switch_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	/* create a switch state */
	ret = iotcon_state_create(&switch_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(switch_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	ret = iotcon_state_set_bool(switch_state, "switch", false);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_bool() Fail(%d)", ret);
		iotcon_state_destroy(switch_state);
		iotcon_representation_destroy(switch_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	/* Set a light state into light Representation */
	ret = iotcon_representation_set_state(switch_repr, switch_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(switch_state);
		iotcon_representation_destroy(switch_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_state_destroy(switch_state);

	ret = iotcon_representation_append_child(room_repr, switch_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_append_child() Fail(%d)", ret);
		iotcon_representation_destroy(switch_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_representation_destroy(switch_repr);

	ret = iotcon_request_get_query(request, &query);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_query() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		return;
	}

	if (query) {
		ret = iotcon_query_lookup(query, "if", &query_str);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_query_lookup() Fail(%d)", ret);
			iotcon_representation_destroy(room_repr);
			return;
		}
	}

	if (query_str && (TEST_STR_EQUAL == strcmp("oic.if.b", query_str))) {
		DBG("operation for BATCH interface");
		iface = IOTCON_INTERFACE_BATCH;
	} else {
		DBG("operation for DEFAULT interface");
		iface = IOTCON_INTERFACE_DEFAULT;
	}

	ret = _send_response(response, room_repr, iface);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(room_repr);
}

static void _request_handler_put(iotcon_request_h request, iotcon_response_h response)
{
	int ret;
	iotcon_representation_h resp_repr;

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	INFO("PUT request");

	/* do PUT operation */

	ret = _send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(resp_repr);
}

static void _request_handler_post(iotcon_response_h response)
{
	int ret;
	iotcon_representation_h resp_repr;

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	INFO("POST request");

	/* do POST operation */

	ret = _send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(resp_repr);
}

static void _request_handler_delete(iotcon_response_h response)
{
	int ret;
	iotcon_representation_h resp_repr;

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	INFO("DELETE request");

	/* do DELETE operation */

	ret = _send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(resp_repr);
}

static void _light_request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
{
	int ret;
	int types;
	iotcon_response_h response;

	RET_IF(NULL == request);

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_types() Fail(%d)", ret);
		return;
	}

	ret = iotcon_response_create(request, &response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_create() Fail");
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

	iotcon_response_destroy(response);
}

static void _room_request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
{
	FN_CALL;
	int ret, types;
	iotcon_response_h response;

	RET_IF(NULL == request);

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_types() Fail(%d)", ret);
		return;
	}

	ret = iotcon_response_create(request, &response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_create() Fail(%d)", ret);
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

	iotcon_response_destroy(response);
}

int main(int argc, char **argv)
{
	FN_CALL;
	int ret;
	GMainLoop *loop;
	iotcon_resource_h room_handle, light_handle;
	iotcon_resource_types_h room_rtypes, light_rtypes;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	ret = iotcon_open();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_open() Fail(%d)", ret);
		return -1;
	}

	/* register room resource */
	ret = iotcon_resource_types_create(&room_rtypes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		iotcon_close();
		return -1;
	}
	ret = iotcon_resource_types_add(room_rtypes, "core.room");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_add(%s) Fail(%d)", "core.room", ret);
		iotcon_resource_types_destroy(room_rtypes);
		iotcon_close();
		return -1;
	}

	ret = iotcon_resource_create("/a/room", room_rtypes,
			(IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_BATCH),
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), _room_request_handler,
			NULL, &room_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_create() Fail(%d)", ret);
		iotcon_resource_types_destroy(room_rtypes);
		iotcon_close();
		return -1;
	}
	iotcon_resource_types_destroy(room_rtypes);

	/* register light resource */
	ret = iotcon_resource_types_create(&light_rtypes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		iotcon_resource_destroy(room_handle);
		iotcon_close();
		return -1;
	}
	ret = iotcon_resource_types_add(light_rtypes, "core.light");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_add(%s) Fail(%d)", "core.light", ret);
		iotcon_resource_types_destroy(light_rtypes);
		iotcon_resource_destroy(room_handle);
		iotcon_close();
		return -1;
	}

	ret = iotcon_resource_create("/a/light", light_rtypes,
			(IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_BATCH),
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), _light_request_handler,
			NULL, &light_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_create() Fail");
		iotcon_resource_types_destroy(light_rtypes);
		iotcon_resource_destroy(room_handle);
		iotcon_close();
		return -1;
	}
	iotcon_resource_types_destroy(light_rtypes);

	ret = iotcon_resource_bind_child_resource(room_handle, light_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_bind_child_resource() Fail");
		iotcon_resource_destroy(light_handle);
		iotcon_resource_destroy(room_handle);
		iotcon_close();
		return -1;
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	iotcon_resource_destroy(light_handle);
	iotcon_resource_destroy(room_handle);

	/* iotcon close */
	iotcon_close();

	return 0;
}
