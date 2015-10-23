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
#include <iotcon-internal.h>
#include "test.h"

/* Door Resource */
typedef struct _door_resource_s {
	bool state;
	char *uri_path;
	char *type;
	int ifaces;
	int properties;
	iotcon_resource_h handle;
	iotcon_observers_h observers;
	iotcon_representation_h repr;
} door_resource_s;

static bool resource_created;

static void _request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data);

static int _set_door_resource(door_resource_s *door)
{
	int ret;

	door->state = false;

	door->uri_path = strdup("/a/door");
	if (NULL == door->uri_path) {
		ERR("strdup(/a/door) Fail");
		return -1;
	}

	door->type = strdup("core.door");
	if (NULL == door->type) {
		ERR("strdup(core.door) Fail");
		free(door->uri_path);
		return -1;
	}

	door->ifaces = IOTCON_INTERFACE_DEFAULT;
	door->properties = IOTCON_DISCOVERABLE;

	ret = iotcon_observers_create(&door->observers);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_observers_create() Fail");
		free(door->type);
		free(door->uri_path);
		return -1;
	}

	return 0;
}

static void _free_door_resource(door_resource_s *door)
{
	iotcon_observers_destroy(door->observers);
	free(door->type);
	free(door->uri_path);
}

static void _check_door_state(door_resource_s door)
{
	if (false == door.state)
		INFO("[Door] closed.");
	else
		INFO("[Door] opened.");
}

static iotcon_resource_h _create_door_resource(char *uri_path, char *type, int ifaces,
		int properties, void *user_data)
{
	int ret;
	iotcon_resource_h handle;
	iotcon_resource_types_h resource_types;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		return NULL;
	}

	ret = iotcon_resource_types_add(resource_types, type);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		ERR("iotcon_resource_types_add() Fail(%d)", ret);
		return NULL;
	}

	/* register door resource */
	ret = iotcon_resource_create(uri_path, resource_types, ifaces, properties,
			_request_handler, user_data, &handle);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		ERR("iotcon_resource_create() Fail");
		return NULL;
	}

	iotcon_resource_types_destroy(resource_types);

	return handle;
}

static int _send_response(iotcon_response_h response, iotcon_representation_h repr,
		iotcon_response_result_e result)
{
	int ret;

	ret = iotcon_response_set_result(response, result);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_set_result() Fail(%d)", ret);
		return -1;
	}

	ret = iotcon_response_set_representation(response, repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_set_representation() Fail(%d)", ret);
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

static void _request_handler_get(door_resource_s *door, iotcon_response_h response)
{
	int ret;
	iotcon_representation_h resp_repr;
	iotcon_state_h resp_state;
	INFO("GET request");

	/* create a door Representation */
	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	/* create a door state */
	ret = iotcon_state_create(&resp_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_representation_set_uri_path(resp_repr, door->uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_uri_path() Fail(%d)", ret);
		iotcon_state_destroy(resp_state);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_set_bool(resp_state, "opened", door->state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_bool() Fail(%d)", ret);
		iotcon_state_destroy(resp_state);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_representation_set_state(resp_repr, resp_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(resp_state);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	iotcon_state_destroy(resp_state);

	ret = _send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_OK);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(resp_repr);
}

static void _request_handler_put(door_resource_s *door, iotcon_request_h request,
		iotcon_response_h response)
{
	int ret;
	bool bval;
	iotcon_representation_h req_repr, resp_repr;
	iotcon_state_h req_state, resp_state;;
	INFO("PUT request");

	ret = iotcon_request_get_representation(request, &req_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_representation() Fail(%d)", ret);
		return;
	}

	ret = iotcon_representation_get_state(req_repr, &req_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_get_state() Fail(%d)", ret);
		return;
	}

	ret = iotcon_state_get_bool(req_state, "opened", &bval);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_get_bool() Fail(%d)", ret);
		return;
	}

	door->state = bval;

	_check_door_state(*door);

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	ret = iotcon_representation_set_uri_path(resp_repr, door->uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_uri_path() Fail(%d)", ret);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_create(&resp_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_set_bool(resp_state, "opened", door->state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_bool() Fail(%d)", ret);
		iotcon_state_destroy(resp_state);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_representation_set_state(resp_repr, resp_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(resp_state);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	iotcon_state_destroy(resp_state);

	ret = _send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_OK);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(resp_repr);
}

static gboolean _notifier(gpointer user_data)
{
	int ret;
	static int i = 0;
	iotcon_notimsg_h msg;
	door_resource_s *door;
	iotcon_representation_h repr;

	door = user_data;

	if ((5 == i++) || !(door->observers))
		return G_SOURCE_REMOVE;

	INFO("NOTIFY!");
	ret = iotcon_representation_create(&repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}

	ret = iotcon_notimsg_create(repr, IOTCON_INTERFACE_DEFAULT, &msg);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_notimsg_create() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return G_SOURCE_REMOVE;
	}

	ret = iotcon_resource_notify(door->handle, msg, door->observers);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_notify() Fail(%d)", ret);
		iotcon_notimsg_destroy(msg);
		iotcon_representation_destroy(repr);
		return G_SOURCE_REMOVE;
	}

	iotcon_notimsg_destroy(msg);
	iotcon_representation_destroy(repr);

	return G_SOURCE_CONTINUE;
}

static void _request_handler_post(door_resource_s *door, iotcon_response_h response)
{
	int ret;
	iotcon_state_h resp_state;
	iotcon_representation_h resp_repr = NULL;
	iotcon_resource_h new_door_handle;
	INFO("POST request");

	if (resource_created) {
		INFO("Resource is already created");
		return;
	}

	new_door_handle = _create_door_resource("/a/door1", door->type,
			IOTCON_INTERFACE_DEFAULT, (IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), NULL);
	if (NULL == new_door_handle) {
		ERR("_create_door_resource() Fail");
		return;
	}
	resource_created = true;

	/* send information that new resource was created */
	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	ret = iotcon_state_create(&resp_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_state_set_str(resp_state, "createduripath", "/a/door1");
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_str() Fail(%d)", ret);
		iotcon_state_destroy(resp_state);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_representation_set_state(resp_repr, resp_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(resp_state);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	iotcon_state_destroy(resp_state);

	ret = _send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_RESOURCE_CREATED);
	if (0 != ret) {
		ERR("_send_response() Fail(%d)", ret);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	iotcon_representation_destroy(resp_repr);

	/* add observe */
	g_timeout_add_seconds(5, _notifier, door);
}

static void _request_handler_delete(iotcon_resource_h resource,
		iotcon_response_h response)
{
	int ret;
	iotcon_representation_h resp_repr = NULL;
	iotcon_response_result_e result = IOTCON_RESPONSE_RESULT_OK;
	INFO("DELETE request");

	ret = iotcon_resource_destroy(resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_unregiser_resource() Fail(%d)", ret);
		return;
	}

	result = IOTCON_RESPONSE_RESULT_RESOURCE_DELETED;

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	ret = _send_response(response, resp_repr, result);
	if (0 != ret)
		ERR("_send_response() Fail(%d)", ret);

	iotcon_representation_destroy(resp_repr);
}

static int _query_cb(const char *key, const char *value, void *user_data)
{
	INFO("key : %s, value : %s", key, value);

	return IOTCON_FUNC_CONTINUE;
}

static void _request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
{
	door_resource_s *door;
	iotcon_query_h query;
	iotcon_response_h response = NULL;
	int ret, types, observer_id, observer_action;
	char *host_address;

	RET_IF(NULL == request);

	door = user_data;

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

	ret = iotcon_response_create(request, &response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_create() Fail(%d)", ret);
		return;
	}

	if (IOTCON_REQUEST_GET & types)
		_request_handler_get(door, response);

	else if (IOTCON_REQUEST_PUT & types)
		_request_handler_put(door, request, response);

	else if (IOTCON_REQUEST_POST & types)
		_request_handler_post(door, response);

	else if (IOTCON_REQUEST_DELETE & types)
		_request_handler_delete(resource, response);

	iotcon_response_destroy(response);

	ret = iotcon_request_get_host_address(request, &host_address);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_host_address() Fail(%d)", ret);
		return;
	}
	INFO("host_address : %s", host_address);

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
			ret = iotcon_observers_add(door->observers, observer_id);
		} else if (IOTCON_OBSERVE_DEREGISTER == observer_action) {
			ret = iotcon_request_get_observer_id(request, &observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_request_get_observer_id() Fail(%d)", ret);
				return;
			}
			ret = iotcon_observers_remove(door->observers, observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_observers_remove() Fail(%d)", ret);
				return;
			}
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
		return G_SOURCE_REMOVE;

	return G_SOURCE_CONTINUE;
}

int main(int argc, char **argv)
{
	FN_CALL;
	int ret;
	GMainLoop *loop;
	door_resource_s my_door = {0};

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	ret = iotcon_open();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_open() Fail(%d)", ret);
		return -1;
	}

	/* set local door resource */
	ret = _set_door_resource(&my_door);
	if (0 != ret) {
		ERR("_set_door_resource() Fail");
		iotcon_close();
		return -1;
	}

	/* add resource options */
	my_door.ifaces |= IOTCON_INTERFACE_BATCH;
	my_door.properties |= IOTCON_OBSERVABLE;

	/* add presence */
	g_timeout_add_seconds(10, _presence_timer, NULL);

	/* create new door resource */
	my_door.handle = _create_door_resource(my_door.uri_path, my_door.type, my_door.ifaces,
			my_door.properties, &my_door);
	if (NULL == my_door.handle) {
		ERR("_create_door_resource() Fail");
		_free_door_resource(&my_door);
		iotcon_close();
		return -1;
	}

	_check_door_state(my_door);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	iotcon_resource_destroy(my_door.handle);

	_free_door_resource(&my_door);

	/* iotcon close */
	iotcon_close();

	return 0;
}
