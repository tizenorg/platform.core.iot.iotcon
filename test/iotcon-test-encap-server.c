/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <glib.h>

#include <iotcon.h>
#include "test.h"

#define DOOR_RESOURCE_URI "/door/1"
#define DOOR_RESOURCE_TYPE "org.tizen.door"

/* Door Resource */
typedef struct _door_resource_s {
	bool state;
	char *uri_path;
	char *type;
	uint8_t policies;
	iotcon_lite_resource_h handle;
} door_resource_s;

static int _set_door_resource(door_resource_s *door)
{
	door->state = false;

	door->uri_path = strdup(DOOR_RESOURCE_URI);
	if (NULL == door->uri_path) {
		ERR("strdup(%s) Fail", DOOR_RESOURCE_URI);
		return -1;
	}

	door->type = strdup(DOOR_RESOURCE_TYPE);
	if (NULL == door->type) {
		ERR("strdup(%s) Fail", DOOR_RESOURCE_TYPE);
		free(door->uri_path);
		return -1;
	}

	door->policies = IOTCON_RESOURCE_DISCOVERABLE;

	return 0;
}

static void _free_door_resource(door_resource_s *door)
{
	free(door->type);
	free(door->uri_path);
}

static void _check_door_attributes(door_resource_s door)
{
	if (false == door.state)
		INFO("[Door] closed.");
	else
		INFO("[Door] opened.");
}

static gboolean _door_attributes_changer(gpointer user_data)
{
	int ret;
	door_resource_s *door = user_data;
	iotcon_attributes_h recv_attributes, send_attributes;

	ret = iotcon_lite_resource_get_attributes(door->handle, &recv_attributes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_lite_resource_get_attributes() Fail(%d)", ret);
		return G_SOURCE_CONTINUE;
	}

	ret = iotcon_attributes_get_bool(recv_attributes, "opened", &(door->state));
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_attributes_get_bool() Fail(%d)", ret);
		return G_SOURCE_CONTINUE;
	}

	if (true == door->state)
		door->state = false;
	else
		door->state = true;

	ret = iotcon_attributes_create(&send_attributes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_attributes_create() Fail(%d)", ret);
		return G_SOURCE_CONTINUE;
	}

	ret = iotcon_attributes_add_bool(send_attributes, "opened", door->state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_attributes_add_bool() Fail(%d)", ret);
		iotcon_attributes_destroy(send_attributes);
		return G_SOURCE_CONTINUE;
	}

	ret = iotcon_lite_resource_update_attributes(door->handle, send_attributes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_lite_resource_update_attributes() Fail(%d)", ret);
		iotcon_attributes_destroy(send_attributes);
		return G_SOURCE_CONTINUE;
	}

	iotcon_attributes_destroy(send_attributes);

	_check_door_attributes(*door);

	return G_SOURCE_CONTINUE;
}

static bool _door_attributes_changed(iotcon_lite_resource_h resource,
		iotcon_attributes_h attributes, void *user_data)
{
	FN_CALL;
	bool opened;
	int ret;

	ret = iotcon_attributes_get_bool(attributes, "opened", &opened);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_attributes_get_bool() Fail(%d)", ret);
		return false;
	}
	DBG("opened: %d", opened);

	return true;
}

static iotcon_lite_resource_h _create_door_resource(char *uri_path, char *type,
		uint8_t policies, void *user_data)
{
	int ret;
	iotcon_attributes_h attributes;
	iotcon_lite_resource_h handle;
	iotcon_resource_types_h resource_types;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		return NULL;
	}

	ret = iotcon_resource_types_add(resource_types, type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_add() Fail(%d)", ret);
		iotcon_resource_types_destroy(resource_types);
		return NULL;
	}

	ret = iotcon_attributes_create(&attributes);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_attributes_create() Fail(%d)", ret);
		iotcon_resource_types_destroy(resource_types);
		return NULL;
	}

	ret = iotcon_attributes_add_bool(attributes, "opened", false);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_attributes_add_bool() Fail(%d)", ret);
		iotcon_attributes_destroy(attributes);
		iotcon_resource_types_destroy(resource_types);
		return NULL;
	}

	/* register door resource */
	ret = iotcon_lite_resource_create(uri_path, resource_types, policies, attributes,
			_door_attributes_changed, NULL, &handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_lite_resource_create() Fail");
		iotcon_attributes_destroy(attributes);
		iotcon_resource_types_destroy(resource_types);
		return NULL;
	}

	iotcon_attributes_destroy(attributes);
	iotcon_resource_types_destroy(resource_types);

	return handle;
}

int main(int argc, char **argv)
{
	FN_CALL;
	int ret;
	GMainLoop *loop;
	door_resource_s my_door = {0};

	loop = g_main_loop_new(NULL, FALSE);

	/* initialize iotcon */
	ret = iotcon_initialize(NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_initialize() Fail(%d)", ret);
		return -1;
	}

	/* start presence */
	ret = iotcon_start_presence(10);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_start_presence() Fail(%d)", ret);
		return -1;
	}

	/* set local door resource */
	ret = _set_door_resource(&my_door);
	if (0 != ret) {
		ERR("_set_door_resource() Fail");
		iotcon_deinitialize();
		return -1;
	}

	/* add resource options */
	my_door.policies |= IOTCON_RESOURCE_OBSERVABLE;

	/* create new door resource */
	my_door.handle = _create_door_resource(my_door.uri_path, my_door.type,
			my_door.policies, &my_door);
	if (NULL == my_door.handle) {
		ERR("_create_door_resource() Fail");
		_free_door_resource(&my_door);
		iotcon_stop_presence();
		iotcon_deinitialize();
		return -1;
	}

	_check_door_attributes(my_door);

	g_timeout_add_seconds(7, _door_attributes_changer, &my_door);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	iotcon_lite_resource_destroy(my_door.handle);

	_free_door_resource(&my_door);

	iotcon_stop_presence();

	/* deinitialize iotcon */
	iotcon_deinitialize();

	return 0;
}
