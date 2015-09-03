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
	char *uri_path;
	char *type;
	iotcon_representation_h repr;
} door_resource_s;

static door_resource_s my_door;
static bool resource_created = false;

static iotcon_observers_h observers = NULL;

static void _request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data);

static int _set_door_resource()
{
	my_door.state = false;
	my_door.type = strdup("core.door");
	if (NULL == my_door.type) {
		ERR("strdup(core.door) Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	my_door.uri_path = strdup("/a/door");
	if (NULL == my_door.uri_path) {
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

static iotcon_resource_h _create_door_resource(char *uri_path,
		iotcon_interface_e interfaces, iotcon_resource_property_e properties)
{
	int ret;
	iotcon_resource_h handle;
	iotcon_resource_types_h resource_types;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		return NULL;
	}

	ret = iotcon_resource_types_insert(resource_types, my_door.type);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		ERR("iotcon_resource_types_insert() Fail(%d)", ret);
		return NULL;
	}

	/* register door resource */
	ret = iotcon_register_resource(uri_path, resource_types,
			interfaces, properties, _request_handler, NULL, &handle);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		ERR("iotcon_register_resource() Fail");
		return NULL;
	}

	iotcon_resource_types_destroy(resource_types);

	return handle;
}

static void _send_response(iotcon_response_h response, iotcon_representation_h repr,
		iotcon_response_result_e result)
{
	iotcon_response_set_result(response, result);
	iotcon_response_set_representation(response, repr);

	/* send Representation to the client */
	iotcon_response_send(response);
}

static void _request_handler_get(iotcon_response_h response)
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
		return;
	}

	iotcon_representation_set_uri_path(resp_repr, my_door.uri_path);
	iotcon_state_set_bool(resp_state, "opened", my_door.state);

	iotcon_representation_set_state(resp_repr, resp_state);

	_send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_OK);

	iotcon_representation_destroy(resp_repr);
}

static void _request_handler_put(iotcon_request_h request, iotcon_response_h response)
{
	bool bval;
	int ret;
	iotcon_representation_h req_repr, resp_repr;
	iotcon_state_h req_state, resp_state;;
	INFO("PUT request");

	ret = iotcon_request_get_representation(request, &req_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_request_get_representation() Fail(%d)", ret);
		return;
	}

	iotcon_representation_get_state(req_repr, &req_state);

	iotcon_state_get_bool(req_state, "opened", &bval);
	my_door.state = bval;

	_check_door_state();

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	ret = iotcon_state_create(&resp_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		return;
	}

	iotcon_representation_set_uri_path(resp_repr, my_door.uri_path);
	iotcon_state_set_bool(resp_state, "opened", my_door.state);

	iotcon_representation_set_state(resp_repr, resp_state);

	_send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_OK);

	iotcon_representation_destroy(resp_repr);
}

static gboolean _notifier(gpointer user_data)
{
	int ret;
	iotcon_representation_h repr;
	iotcon_notimsg_h msg;

	static int i = 0;
	if ((5 == i++) || !(observers))
		return FALSE;

	INFO("NOTIFY!");
	ret = iotcon_representation_create(&repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return FALSE;
	}

	ret = iotcon_notimsg_create(repr, IOTCON_INTERFACE_DEFAULT, &msg);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_notimsg_create() Fail(%d)", ret);
		return FALSE;
	}

	iotcon_notify_list_of_observers(user_data, msg, observers);

	iotcon_notimsg_destroy(msg);
	iotcon_representation_destroy(repr);

	return TRUE;
}

static void _request_handler_post(iotcon_resource_h resource, iotcon_response_h response)
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

	new_door_handle = _create_door_resource("/a/door1", IOTCON_INTERFACE_DEFAULT,
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE));
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
		return;
	}

	iotcon_state_set_str(resp_state, "createduripath", "/a/door1");

	iotcon_representation_set_state(resp_repr, resp_state);

	_send_response(response, resp_repr, IOTCON_RESPONSE_RESULT_RESOURCE_CREATED);

	iotcon_representation_destroy(resp_repr);

	/* add observe */
	g_timeout_add_seconds(5, _notifier, resource);
}

static void _request_handler_delete(iotcon_resource_h resource,
		iotcon_response_h response)
{
	int ret;
	iotcon_representation_h resp_repr = NULL;
	iotcon_response_result_e result = IOTCON_RESPONSE_RESULT_OK;
	INFO("DELETE request");

	iotcon_unregister_resource(resource);
	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	result = IOTCON_RESPONSE_RESULT_RESOURCE_DELETED;

	_send_response(response, resp_repr, result);

	iotcon_representation_destroy(resp_repr);
}

static int _query_cb(const char *key, const char *value, void *user_data)
{
	INFO("key : %s", key);
	INFO("value : %s", value);

	return IOTCON_FUNC_CONTINUE;
}

static void _request_handler(iotcon_resource_h resource, iotcon_request_h request,
		void *user_data)
{
	iotcon_query_h query;
	iotcon_response_h response = NULL;
	int ret, types, observer_id, observer_action;

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

	ret = iotcon_response_create(request, &response);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_create() Fail(%d)", ret);
		return;
	}

	if (IOTCON_REQUEST_GET & types)
		_request_handler_get(response);

	else if (IOTCON_REQUEST_PUT & types)
		_request_handler_put(request, response);

	else if (IOTCON_REQUEST_POST & types)
		_request_handler_post(resource, response);

	else if (IOTCON_REQUEST_DELETE & types)
		_request_handler_delete(resource, response);

	iotcon_response_destroy(response);

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
			ret = iotcon_observers_append(observers, observer_id, &observers);
		} else if (IOTCON_OBSERVE_DEREGISTER == observer_action) {
			ret = iotcon_request_get_observer_id(request, &observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_request_get_observer_id() Fail(%d)", ret);
				return;
			}
			ret = iotcon_observers_remove(observers, observer_id, &observers);
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
		return FALSE;

	return TRUE;
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	iotcon_resource_h door_handle;
	iotcon_interface_e door_interfaces = IOTCON_INTERFACE_DEFAULT;
	iotcon_resource_property_e resource_properties = IOTCON_DISCOVERABLE;
	int ret = IOTCON_ERROR_NONE;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	iotcon_open();

	/* set local door resource */
	ret = _set_door_resource();
	if (IOTCON_ERROR_NONE != ret) {
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

	/* iotcon close */
	iotcon_close();

	return 0;
}
