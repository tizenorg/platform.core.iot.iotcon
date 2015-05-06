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

#include <iotcon.h>
#include "test-log.h"

const char* const door_uri = "/a/door";
const char* const door_type = "core.door";

typedef struct _door_resource_s {
	bool state;
	char *uri;
	char *type;
	iotcon_repr_h repr;
} door_resource_s;

static door_resource_s my_door;

static void _set_door_resource(door_resource_s *door_s)
{
	door_s->state = false;
	door_s->type = strdup(door_type);
	door_s->uri = strdup(door_uri);
}

static void _entity_handler_door(const iotcon_request_s *request_s)
{
	FN_CALL;

	RET_IF(NULL == request_s);

	char *requestType = request_s->request_type;

	int requestFlag = request_s->request_handler_flag;
	if (requestFlag & IOTCON_REQUEST_FLAG) {
		iotcon_response_h pResponse = iotcon_response_new(request_s->request_handle,
				request_s->resource_handle);
		if (NULL == pResponse) {
			ERR("pResponse is NULL");
			return;
		}

		if (!strcmp("GET", requestType)) {
			INFO("GET request");

			iotcon_repr_h door_rep = iotcon_repr_new();
			iotcon_repr_set_uri(door_rep, my_door.uri);
			iotcon_repr_set_bool(door_rep, "opened", my_door.state);

			iotcon_response_set(pResponse, IOTCON_RESP_REPRESENTATION, door_rep);
			iotcon_response_set(pResponse, IOTCON_RESP_ERR_CODE, 200);
			iotcon_response_set(pResponse, IOTCON_RESP_RESULT, IOTCON_EH_OK);
			iotcon_send_resource_response(pResponse);

		}
		else if (!strcmp("PUT", requestType)) {
			INFO("PUT request");

			iotcon_repr_h repr = iotcon_request_get_representation(request_s);
			my_door.state = iotcon_repr_get_bool(repr, "opened");
			iotcon_repr_h door_rep = iotcon_repr_new();
			iotcon_repr_set_uri(door_rep, my_door.uri);
			iotcon_repr_set_bool(door_rep, "opened", my_door.state);

			iotcon_response_set(pResponse, IOTCON_RESP_REPRESENTATION, door_rep);
			iotcon_response_set(pResponse, IOTCON_RESP_ERR_CODE, 200);
			iotcon_response_set(pResponse, IOTCON_RESP_RESULT, IOTCON_EH_OK);
			iotcon_send_resource_response(pResponse);

		}
	}
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	int result = 0;

	loop = g_main_loop_new(NULL, FALSE);

	/* initialize address and port */
	iotcon_initialize("0.0.0.0", 0);

	_set_door_resource(&my_door);

	/* register door resource */
	iotcon_resource_h door_handle = iotcon_register_resource(door_uri,
			door_type, IOTCON_INTERFACE_DEFAULT,
			(IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE), _entity_handler_door);
	if (0 != result) {
		ERR("register %s resource Fail(%d)", "/a/light", result);
		return result;
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	iotcon_unregister_resource(door_handle);

	return 0;
}
