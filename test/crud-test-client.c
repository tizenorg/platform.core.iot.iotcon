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
#include <stdbool.h>
#include <stdlib.h>
#include <glib.h>

#include <iotcon.h>
#include "test.h"

static char *door_resource_sid;
static GList *resource_sid_list;

static const char* const door_uri_path = "/a/door";

static void _on_observe(iotcon_client_h resource,
		iotcon_representation_h recv_repr,
		iotcon_options_h header_options,
		int response_result,
		int sequence_number,
		void *user_data)
{
	INFO("_on_observe");

	static int i = 0;
	i++;

	if (2 == i)
		iotcon_observer_stop(resource);
}

static void _on_delete(iotcon_client_h resource, iotcon_options_h header_options,
		int response_result, void *user_data)
{
	int ret;
	iotcon_client_h door_resource = user_data;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result
			&& IOTCON_RESPONSE_RESULT_RESOURCE_DELETED != response_result,
			"_on_delete Response error(%d)", response_result);
	INFO("DELETE request was successful");

	/* delete callback operations */

	ret = iotcon_observer_start(door_resource, IOTCON_OBSERVE_ALL, NULL, _on_observe,
			NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_observer_start() Fail(%d)", ret);

	iotcon_client_destroy(door_resource);
}

static void _on_post(iotcon_client_h resource, iotcon_representation_h recv_repr,
		iotcon_options_h header_options, int response_result, void *user_data)
{
	int ret, ifaces = 0;
	iotcon_state_h recv_state;
	char *host, *created_uri_path;
	iotcon_resource_types_h types = NULL;
	iotcon_client_h new_door_resource, door_resource;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result
			&& IOTCON_RESPONSE_RESULT_RESOURCE_CREATED != response_result,
			"_on_post Response error(%d)", response_result);
	INFO("POST request was successful");

	ret = iotcon_representation_get_state(recv_repr, &recv_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_get_state() Fail(%d)", ret);
		return;
	}

	ret = iotcon_state_get_str(recv_state, "createduripath", &created_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_get_str() Fail(%d)", ret);
		return;
	}
	DBG("New resource created : %s", created_uri_path);

	ret = iotcon_client_get_host(resource, &host);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_host() Fail(%d)", ret);
		return;
	}

	ret = iotcon_client_get_types(resource, &types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_types() Fail(%d)", ret);
		return;
	}

	ret = iotcon_client_get_interfaces(resource, &ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_ifaces() Fail(%d)", ret);
		return;
	}

	ret = iotcon_client_create(host, created_uri_path, true, types, ifaces,
			&new_door_resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_create() Fail(%d)", ret);
		return;
	}

	ret = iotcon_client_ref(resource, &door_resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_ref() Fail(%d)", ret);
		iotcon_client_destroy(new_door_resource);
		return;
	}

	ret = iotcon_delete(new_door_resource, _on_delete, door_resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_delete() Fail(%d)", ret);
		iotcon_client_destroy(door_resource);
		iotcon_client_destroy(new_door_resource);
		return;
	}

	iotcon_client_destroy(new_door_resource);
}

static void _on_put(iotcon_client_h resource, iotcon_representation_h recv_repr,
		iotcon_options_h header_options, int response_result, void *user_data)
{
	int ret;
	iotcon_representation_h send_repr;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result, "_on_put Response error(%d)",
			response_result);
	INFO("PUT request was successful");

	ret = iotcon_representation_create(&send_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	/* send POST request */
	ret = iotcon_post(resource, send_repr, NULL, _on_post, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_post() Fail(%d)", ret);

	iotcon_representation_destroy(send_repr);
}

static void _on_get(iotcon_client_h resource, iotcon_representation_h recv_repr,
		iotcon_options_h header_options, int response_result, void *user_data)
{
	int ret;
	bool opened = true;
	iotcon_representation_h send_repr;
	iotcon_state_h send_state;
	iotcon_state_h recv_state = NULL;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result, "_on_get Response error(%d)",
			response_result);
	INFO("GET request was successful");

	ret = iotcon_representation_get_state(recv_repr, &recv_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_get_state() Fail(%d)", ret);
		return;
	}

	ret = iotcon_state_get_bool(recv_state, "opened", &opened);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_get_bool() Fail(%d)", ret);
		return;
	}

	ret = iotcon_representation_create(&send_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return;
	}

	ret = iotcon_state_create(&send_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(send_repr);
		return;
	}

	ret = iotcon_state_set_bool(send_state, "opened", true);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_set_bool() Fail(%d)", ret);
		iotcon_state_destroy(send_state);
		iotcon_representation_destroy(send_repr);
		return;
	}

	ret = iotcon_representation_set_state(send_repr, send_state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(send_state);
		iotcon_representation_destroy(send_repr);
		return;
	}

	iotcon_state_destroy(send_state);

	/* send PUT request */
	ret = iotcon_put(resource, send_repr, NULL, _on_put, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_put() Fail(%d)", ret);

	iotcon_representation_destroy(send_repr);
}

static int _get_res_type_cb(const char *string, void *user_data)
{
	char *resource_uri_path = user_data;

	DBG("[%s] resource type : %s", resource_uri_path, string);

	return IOTCON_FUNC_CONTINUE;
}

static void _presence_handler(int result, unsigned int nonce,
		const char *host_address, void *user_data)
{
	INFO("_presence_handler");
	INFO("result : %d", result);
	INFO("nonce : %d", nonce);
	INFO("host_address : %s", host_address);
}

static int _sid_compare(const void *a, const void *b)
{
	return strcmp(a, b);
}

static void _found_resource(iotcon_client_h resource, void *user_data)
{
	int ret;
	GList *node;
	char *resource_sid;
	char *resource_host;
	char *resource_uri_path;
	int resource_interfaces;
	iotcon_presence_h presence_handle;
	iotcon_resource_types_h resource_types;

	if (NULL == resource)
		return;

	INFO("===== resource found =====");

	/* get the resource URI */
	ret = iotcon_client_get_uri_path(resource, &resource_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_uri_path() Fail(%d)", ret);
		return;
	}

	/* get the resource unique id.
	 * this is unique per-server independent on how it was discovered. */
	ret = iotcon_client_get_server_id(resource, &resource_sid);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_server_id() Fail(%d)", ret);
		return;
	}
	DBG("[%s] resource server id : %s", resource_uri_path, resource_sid);

	node = g_list_find_custom(resource_sid_list, resource_sid, _sid_compare);

	if (node && TEST_STR_EQUAL == strcmp(door_uri_path, resource_uri_path)) {
		DBG("uri_path \"%s\" already found. skip !", resource_uri_path);
		return;
	}

	door_resource_sid = strdup(resource_sid);
	if (NULL == door_resource_sid) {
		ERR("strdup(door_resource_sid) Fail");
		return;
	}

	resource_sid_list = g_list_append(resource_sid_list, door_resource_sid);

	/* get the resource host address */
	ret = iotcon_client_get_host(resource, &resource_host);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_host() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, door_resource_sid);
		free(door_resource_sid);
		return;
	}
	DBG("[%s] resource host : %s", resource_uri_path, resource_host);

	/* get the resource interfaces */
	ret = iotcon_client_get_interfaces(resource, &resource_interfaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_interfaces() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, door_resource_sid);
		free(door_resource_sid);
		return;
	}
	if (IOTCON_INTERFACE_DEFAULT & resource_interfaces)
		DBG("[%s] resource interface : DEFAULT_INTERFACE", resource_uri_path);
	if (IOTCON_INTERFACE_LINK & resource_interfaces)
		DBG("[%s] resource interface : LINK_INTERFACE", resource_uri_path);
	if (IOTCON_INTERFACE_BATCH & resource_interfaces)
		DBG("[%s] resource interface : BATCH_INTERFACE", resource_uri_path);
	if (IOTCON_INTERFACE_GROUP & resource_interfaces)
		DBG("[%s] resource interface : GROUP_INTERFACE", resource_uri_path);

	/* get the resource types */
	ret = iotcon_client_get_types(resource, &resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_types() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, door_resource_sid);
		free(door_resource_sid);
		return;
	}

	ret = iotcon_resource_types_foreach(resource_types, _get_res_type_cb,
			resource_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_foreach() Fail(%d)", ret);
		resource_sid_list = g_list_remove(resource_sid_list, door_resource_sid);
		free(door_resource_sid);
		return;
	}

	iotcon_subscribe_presence(resource_host, "core.door", _presence_handler, NULL,
			&presence_handle);

	if (TEST_STR_EQUAL == strcmp(door_uri_path, resource_uri_path)) {
		iotcon_query_h query;

		ret = iotcon_query_create(&query);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_query_create() Fail(%d)", ret);
			resource_sid_list = g_list_remove(resource_sid_list, door_resource_sid);
			free(door_resource_sid);
			return;
		}

		ret = iotcon_query_insert(query, "query_key", "query_value");
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_query_insert() Fail(%d)", ret);
			iotcon_query_destroy(query);
			resource_sid_list = g_list_remove(resource_sid_list, door_resource_sid);
			free(door_resource_sid);
			return;
		}

		/* send GET Request */
		ret = iotcon_get(resource, query, _on_get, NULL);
		if (IOTCON_ERROR_NONE != ret)
			ERR("iotcon_get() Fail(%d)", ret);

		iotcon_query_destroy(query);
	}

	resource_sid_list = g_list_remove(resource_sid_list, door_resource_sid);
	free(door_resource_sid);
}

int main(int argc, char **argv)
{
	FN_CALL;
	int ret;
	GMainLoop *loop;

	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	ret = iotcon_open();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_open() Fail(%d)", ret);
		return -1;
	}

	/* find door typed resources */
	ret = iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, "core.door", &_found_resource,
			NULL);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_find_resource() Fail(%d)", ret);
		iotcon_close();
		return -1;
	}

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	g_list_free_full(resource_sid_list, free);

	/* iotcon close */
	iotcon_close();

	return 0;
}
