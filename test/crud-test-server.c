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
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	my_door.uri = strdup("/a/door");
	if (NULL == my_door.uri) {
		ERR("strdup(/a/door) Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
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
	iotcon_resource_types_h resource_types = iotcon_resource_types_new();
	if (NULL == resource_types) {
		ERR("iotcon_resource_types_new() Fail");
		return NULL;
	}

	int ret = iotcon_resource_types_insert(resource_types, my_door.type);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(resource_types);
		ERR("iotcon_resource_types_insert() Fail(%d)", ret);
		return NULL;
	}

	/* register door resource */
	iotcon_resource_h handle = iotcon_register_resource(uri, resource_types,
			interfaces, properties, _request_handler, NULL);
	if (NULL == handle) {
		iotcon_resource_types_free(resource_types);
		ERR("iotcon_register_resource() Fail");
		return NULL;
	}

	iotcon_resource_types_free(resource_types);

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

	iotcon_repr_free(resp_repr);
}

static void _request_handler_put(iotcon_request_h request, iotcon_response_h response)
{
	bool bval;
	int ret;
	iotcon_repr_h req_repr = NULL;
	iotcon_repr_h resp_repr = NULL;
	INFO("PUT request");

	ret = iotcon_request_get_representation(request, &req_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_representation() Fail(%d)", ret);
		return;
	}
	iotcon_repr_get_bool(req_repr, "opened", &bval);
	my_door.state = bval;

	_check_door_state();

	resp_repr = iotcon_repr_new();
	iotcon_repr_set_uri(resp_repr, my_door.uri);
	iotcon_repr_set_bool(resp_repr, "opened", my_door.state);

	_send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_OK);

	iotcon_repr_free(resp_repr);
}

static void _request_handler_post(iotcon_response_h response)
{
	iotcon_repr_h resp_repr = NULL;
	INFO("POST request");

	if (resource_created) {
		INFO("Resource is already created");
		return;
	}

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

	iotcon_repr_free(resp_repr);
}

static gboolean _notifier(gpointer user_data)
{
	static int i = 0;
	if ((5 == i++) || !(observers))
		return FALSE;

	INFO("NOTIFY!");
	iotcon_repr_h repr = iotcon_repr_new();
	iotcon_notimsg_h msg = iotcon_notimsg_new(repr, IOTCON_INTERFACE_DEFAULT);
	iotcon_notify_list_of_observers(user_data, msg, observers);

	iotcon_repr_free(repr);

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

	iotcon_repr_free(resp_repr);
}

static int _query_cb(const char *key, const char *value, void *user_data)
{
	INFO("key : %s", key);
	INFO("value : %s", value);

	return IOTCON_FUNC_CONTINUE;
}

static void _request_handler(iotcon_request_h request, void *user_data)
{
	int ret;
	int types;
	iotcon_query_h query;
	iotcon_response_h response = NULL;
	iotcon_observe_action_e observer_action;
	int observer_id;

	RET_IF(NULL == request);

	ret = iotcon_request_get_query(request, &query);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_query() Fail(%d)", ret);
		return;
	}
	if (query)
		iotcon_query_foreach(query, _query_cb, NULL);

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_types() Fail(%d)", ret);
		return;
	}

	response = iotcon_response_new(request);
	if (NULL == response) {
		ERR("iotcon_response_new() Fail(NULL == response)");
		return;
	}

	if (IOTCON_REQUEST_GET & types)
		_request_handler_get(response);

	else if (IOTCON_REQUEST_PUT & types)
		_request_handler_put(request, response);

	else if (IOTCON_REQUEST_POST & types)
		_request_handler_post(response);

	else if (IOTCON_REQUEST_DELETE & types)
		_request_handler_delete(response);

	iotcon_response_free(response);

	if (IOTCON_REQUEST_OBSERVE & types) {
		ret = iotcon_request_get_observer_action(request, &observer_action);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_request_get_observer_action() Fail(%d)", ret);
			return;
		}

		if (IOTCON_OBSERVE_REGISTER == observer_action) {
			ret = iotcon_request_get_observer_id(request, &observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_request_get_observer_id() Fail(%d)", ret);
				return;
			}
			observers = iotcon_observers_append(observers, observer_id);
		} else if (IOTCON_OBSERVE_DEREGISTER == observer_action) {
			ret = iotcon_request_get_observer_id(request, &observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_request_get_observer_id() Fail(%d)", ret);
				return;
			}
			observers = iotcon_observers_remove(observers, observer_id);
		}
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

	/* iotcon initialize */
	iotcon_initialize();

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

	/* iotcon deinitialize */
	iotcon_deinitialize();

	return 0;
}
