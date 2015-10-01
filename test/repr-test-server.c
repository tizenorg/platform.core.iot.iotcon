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

static void _room_request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data);

static void _send_response(iotcon_response_h response, iotcon_representation_h repr,
		iotcon_interface_e interface)
{
	iotcon_response_set_representation(response, repr);
	iotcon_response_set_interface(response, interface);
	iotcon_response_set_result(response, IOTCON_RESPONSE_RESULT_OK);

	/* send Representation to the client */
	iotcon_response_send(response);
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

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_representation_destroy(resp_repr);
}

static int _query_foreach_cb(const char *key, const char *value, void *user_data)
{
	char **interface_str = user_data;

	if (TEST_STR_EQUAL == strcmp("if", key))
		*interface_str = (char*)value;

	return IOTCON_FUNC_CONTINUE;
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

	iotcon_interface_e interface;

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

	iotcon_representation_set_uri_path(room_repr, "/a/room");
	iotcon_state_set_str(room_state, "name", "Michael's Room");

	/* set null */
	iotcon_state_set_null(room_state, "null value");

	ret = iotcon_list_create(IOTCON_TYPE_INT, &temperature_list);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_create() Fail(%d)", ret);
		iotcon_state_destroy(room_state);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_list_insert_int(temperature_list, 22, -1);
	iotcon_list_insert_int(temperature_list, 23, -1);
	iotcon_list_insert_int(temperature_list, 24, -1);
	iotcon_list_insert_int(temperature_list, 25, -1);
	iotcon_list_insert_int(temperature_list, 26, -1);
	iotcon_state_set_list(room_state, "today_temp", temperature_list);

	/* Set a room state into room Representation */
	iotcon_representation_set_state(room_repr, room_state);

	iotcon_list_destroy(temperature_list);
	iotcon_state_destroy(room_state);

	/* create a light Representation */
	ret = iotcon_representation_create(&light_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		return;
	}

	/* create a light state */
	ret = iotcon_state_create(&light_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(light_repr);
		iotcon_representation_destroy(room_repr);
		return;
	}

	iotcon_representation_set_uri_path(light_repr, "/a/light");
	iotcon_state_set_int(light_state, "brightness", 50);

	/* Set a light state into light Representation */
	iotcon_representation_set_state(light_repr, light_state);

	iotcon_representation_append_child(room_repr, light_repr);

	iotcon_state_destroy(light_state);
	iotcon_representation_destroy(light_repr);

	/* create a switch Representation */
	ret = iotcon_representation_create(&switch_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		return;
	}

	/* create a switch state */
	ret = iotcon_state_create(&switch_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		iotcon_representation_destroy(switch_repr);
		return;
	}

	iotcon_representation_set_uri_path(switch_repr, "/a/switch");
	iotcon_state_set_bool(switch_state, "switch", false);

	/* Set a light state into light Representation */
	iotcon_representation_set_state(switch_repr, switch_state);
	iotcon_representation_append_child(room_repr, switch_repr);

	iotcon_state_destroy(switch_state);
	iotcon_representation_destroy(switch_repr);

	ret = iotcon_request_get_query(request, &query);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_query() Fail(%d)", ret);
		iotcon_representation_destroy(room_repr);
		return;
	}
	if (query)
		iotcon_query_foreach(query, _query_foreach_cb, &query_str);

	if (query_str && (TEST_STR_EQUAL == strcmp("oic.if.b", query_str))) {
		DBG("operation for BATCH interface");
		interface = IOTCON_INTERFACE_BATCH;
	} else {
		DBG("operation for DEFAULT interface");
		interface = IOTCON_INTERFACE_DEFAULT;
	}

	_send_response(response, room_repr, interface);
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

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
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

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
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

	_send_response(response, resp_repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_representation_destroy(resp_repr);

}

static void _light_request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
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
	iotcon_open();

	/* register room resource */
	ret = iotcon_resource_types_create(&room_rtypes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		return -1;
	}

	iotcon_resource_types_insert(room_rtypes, "core.room");
	ret = iotcon_register_resource("/a/room", room_rtypes,
			(IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_BATCH),
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), _room_request_handler,
			NULL, &room_handle);
	if (NULL == room_handle) {
		ERR("iotcon_register_resource() Fail");
		iotcon_resource_types_destroy(room_rtypes);
		return -1;
	}

	/* register light resource */
	ret = iotcon_resource_types_create(&light_rtypes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		iotcon_resource_types_destroy(room_rtypes);
		iotcon_unregister_resource(room_handle);
		return -1;
	}

	iotcon_resource_types_insert(light_rtypes, "core.light");
	ret = iotcon_register_resource("/a/light", light_rtypes,
			(IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_BATCH),
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), _light_request_handler,
			NULL, &light_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_register_resource() Fail");
		iotcon_resource_types_destroy(light_rtypes);
		iotcon_resource_types_destroy(room_rtypes);
		iotcon_unregister_resource(room_handle);
		return -1;
	}

	ret = iotcon_resource_bind_child_resource(room_handle, light_handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_bind_child_resource() Fail");
		iotcon_resource_types_destroy(light_rtypes);
		iotcon_unregister_resource(light_handle);
		iotcon_resource_types_destroy(room_rtypes);
		iotcon_unregister_resource(room_handle);
		return -1;
	}

	iotcon_resource_types_destroy(light_rtypes);
	iotcon_resource_types_destroy(room_rtypes);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	iotcon_unregister_resource(light_handle);
	iotcon_unregister_resource(room_handle);

	/* iotcon close */
	iotcon_close();

	return 0;
}
