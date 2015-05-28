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

/* Door Resource */
typedef struct _door_resource_s {
	bool state;
	char *uri;
	char *type;
	iotcon_repr_h repr;
} door_resource_s;

static door_resource_s my_door;
static bool resource_created = false;

iotcon_resource_h door_handle;
iotcon_resource_h new_door_handle;

iotcon_observers_h observers = NULL;

static void _request_handler(iotcon_request_h request, void *user_data);

static iotcon_error_e _set_door_resource()
{
	my_door.state = false;
	my_door.type = strdup("core.door");
	if (NULL == my_door.type) {
		ERR("strdup(core.door) Fail");
		return IOTCON_ERROR_MEMORY;
	}

	my_door.uri = strdup("/a/door");
	if (NULL == my_door.uri) {
		ERR("strdup(/a/door) Fail");
		return IOTCON_ERROR_MEMORY;
	}

	return IOTCON_ERROR_NONE;
}

static void _check_door_state()
{
	if (false == my_door.state)
		INFO("[Door] closed.");
	else
		INFO("[Door] opened.");
}

static iotcon_resource_h _create_door_resource(char *uri, iotcon_interface_e interfaces,
		iotcon_resource_property_e properties)
{
	iotcon_str_list_s resource_types = {my_door.type, NULL};

	/* register door resource */
	iotcon_resource_h handle = iotcon_register_resource(uri, &resource_types,
			interfaces, properties, _request_handler, NULL);
	if (NULL == handle) {
		ERR("iotcon_register_resource() Fail");
		return NULL;
	}

	return handle;
}

static void _send_response(iotcon_response_h response, iotcon_repr_h repr,
		iotcon_response_result_e result)
{
	iotcon_response_set(response, IOTCON_RESPONSE_RESULT, result);
	iotcon_response_set(response, IOTCON_RESPONSE_REPRESENTATION, repr);

	/* send Representation to the client */
	iotcon_response_send(response);
}

static void _request_handler_get(iotcon_response_h response)
{
	iotcon_repr_h resp_repr = NULL;
	INFO("GET request");

	/* create a door Representation */
	resp_repr = iotcon_repr_new();
	iotcon_repr_set_uri(resp_repr, my_door.uri);
	iotcon_repr_set_bool(resp_repr, "opened", my_door.state);

	_send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_OK);
}

static void _request_handler_put(iotcon_request_h request, iotcon_response_h response)
{
	iotcon_repr_h req_repr = NULL;
	iotcon_repr_h resp_repr = NULL;
	INFO("PUT request");

	req_repr = iotcon_request_get_representation(request);
	my_door.state = iotcon_repr_get_bool(req_repr, "opened");

	_check_door_state();

	resp_repr = iotcon_repr_new();
	iotcon_repr_set_uri(resp_repr, my_door.uri);
	iotcon_repr_set_bool(resp_repr, "opened", my_door.state);

	_send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_OK);
}

static void _request_handler_post(iotcon_response_h response)
{
	iotcon_repr_h resp_repr = NULL;
	INFO("POST request");

	if (false == resource_created) {
		new_door_handle = _create_door_resource("/a/door1", IOTCON_INTERFACE_DEFAULT,
				(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE));
		if (NULL == new_door_handle) {
			ERR("_create_door_resource() Fail");
			return;
		}
		resource_created = true;

		/* send information that new resource was created */
		resp_repr = iotcon_repr_new();
		iotcon_repr_set_str(resp_repr, "createduri", "/a/door1");

		_send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_RESOURCE_CREATED);
	}

}

static gboolean _notifier(gpointer user_data)
{
	static int i = 0;
	if ((5 == i++) && !(observers))
		return FALSE;

	INFO("NOTIFY!");
	iotcon_repr_h repr = iotcon_repr_new();
	iotcon_notimsg_h msg = iotcon_notimsg_new(repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_notify(user_data, msg, observers);

	return TRUE;
}

static void _request_handler_delete(iotcon_response_h response)
{
	iotcon_repr_h resp_repr = NULL;
	iotcon_response_result_e result = IOTCON_RESPONSE_RESULT_OK;
	INFO("DELETE request");

	iotcon_unregister_resource(new_door_handle);
	resp_repr = iotcon_repr_new();
	result = IOTCON_RESPONSE_RESULT_RESOURCE_DELETED;

	_send_response(response, resp_repr, result);

	/* add observe */
	g_timeout_add_seconds(5, _notifier, door_handle);
}

static void query_cb(const char *key, const char *value, void *user_data)
{
	INFO("key : %s", key);
	INFO("value : %s", value);
}

static void _request_handler(iotcon_request_h request, void *user_data)
{
	const char *request_type = NULL;
	int request_flag = IOTCON_INIT_FLAG;
	iotcon_query_h query = NULL;
	iotcon_response_h response = NULL;
	FN_CALL;

	RET_IF(NULL == request);

	query = iotcon_request_get_query(request);
	if (query)
		iotcon_query_foreach(query, query_cb, NULL);

	request_type = iotcon_request_get_request_type(request);
	if (NULL == request_type) {
		ERR("request_type is NULL");
		return;
	}

	request_flag = iotcon_request_get_request_handler_flag(request);
	if (request_flag & IOTCON_CRUD_FLAG) {
		response = iotcon_response_new(request);
		if (NULL == response) {
			ERR("iotcon_response_new() Fail(NULL == response)");
			return;
		}

		if (!strcmp("GET", request_type))
			_request_handler_get(response);

		else if (!strcmp("PUT", request_type))
			_request_handler_put(request, response);

		else if (!strcmp("POST", request_type))
			_request_handler_post(response);

		else if (!strcmp("DELETE", request_type))
			_request_handler_delete(response);

		iotcon_response_free(response);
	}
	if (request_flag & IOTCON_OBSERVE_FLAG) {
		if (IOTCON_OBSERVE_REGISTER == iotcon_request_get_observer_action(request))
			observers = iotcon_observers_append(observers,
					iotcon_request_get_observer_id(request));
	}
}

static gboolean _presence_timer(gpointer user_data)
{
	static int i = 0;
	i++;
	if (i % 2)
		iotcon_start_presence(10);
	else
		iotcon_stop_presence();

	if (4 == i)
		return FALSE;

	return TRUE;
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	iotcon_interface_e door_interfaces = IOTCON_INTERFACE_DEFAULT;
	iotcon_resource_property_e resource_properties = IOTCON_DISCOVERABLE;
	iotcon_error_e iotcon_error = IOTCON_ERROR_NONE;

	loop = g_main_loop_new(NULL, FALSE);

	/* initialize address and port */
	iotcon_initialize(IOTCON_ALL_INTERFACES, IOTCON_RANDOM_PORT);

	/* set local door resource */
	iotcon_error = _set_door_resource();
	if (IOTCON_ERROR_NONE != iotcon_error) {
		ERR("_set_door_resource() Fail");
		return -1;
	}

	/* add resource options */
	door_interfaces |= IOTCON_INTERFACE_BATCH;
	resource_properties |= IOTCON_OBSERVABLE;

	/* add presence */
	g_timeout_add_seconds(10, _presence_timer, NULL);

	/* create new door resource */
	door_handle = _create_door_resource("/a/door", door_interfaces, resource_properties);
	if (NULL == door_handle) {
		ERR("_create_door_resource() Fail");
		return -1;
	}

	_check_door_state();

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	iotcon_unregister_resource(door_handle);

	return 0;
}
