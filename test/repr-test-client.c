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
#include <glib.h>
#include <iotcon.h>
#include "test.h"

static const char* const room_uri_path = "/a/room";
static char *room_resource_sid;

static int _get_int_list_cb(int pos, const int value, void *user_data)
{
	DBG("%d°C", value);

	return IOTCON_FUNC_CONTINUE;
}

static void _on_get(iotcon_representation_h recv_repr, int response_result)
{
	int i, ret;
	bool is_null;
	iotcon_list_h list;
	const char *uri_path;
	iotcon_representation_h child_repr;
	iotcon_state_h recv_state, child_state;
	unsigned int key_count, children_count;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result,
			"_on_get Response error(%d)", response_result);
	INFO("GET request was successful");

	DBG("[ parent representation ]");
	iotcon_representation_get_uri_path(recv_repr, &uri_path);
	if (uri_path)
		DBG("uri_path : %s", uri_path);

	iotcon_representation_get_state(recv_repr, &recv_state);

	ret = iotcon_state_get_keys_count(recv_state, &key_count);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_get_keys_count() Fail(%d)", ret);
		return;
	}

	if (key_count) {
		char *str;
		iotcon_state_get_str(recv_state, "name", &str);
		if (str)
			DBG("name : %s", str);

		iotcon_state_get_list(recv_state, "today_temp", &list);

		DBG("today's temperature :");
		iotcon_list_foreach_int(list, _get_int_list_cb, NULL);

		ret = iotcon_state_is_null(recv_state, "null value", &is_null);
		if (is_null)
			DBG("null value is null");
	}

	ret = iotcon_representation_get_children_count(recv_repr, &children_count);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_get_children_count() Fail(%d)", ret);
		return;
	}

	for (i = 0; i < children_count; i++) {
		DBG("[ child representation ]");
		const char *uri_path;

		ret = iotcon_representation_get_nth_child(recv_repr, i, &child_repr);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_get_nth_child(%d) Fail(%d)", i, ret);
			continue;
		}

		iotcon_representation_get_uri_path(child_repr, &uri_path);
		if (NULL == uri_path)
			continue;

		DBG("uri_path : %s", uri_path);

		iotcon_representation_get_state(child_repr, &child_state);
		if (TEST_STR_EQUAL == strcmp("/a/light", uri_path)) {
			ret = iotcon_state_get_keys_count(child_state, &key_count);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_get_keys_count() Fail(%d)", ret);
				return;
			}

			if (key_count) {
				int brightness;
				iotcon_state_get_int(child_state, "brightness", &brightness);
				DBG("brightness : %d", brightness);
			}
		} else if (TEST_STR_EQUAL == strcmp("/a/switch", uri_path)) {
			ret = iotcon_state_get_keys_count(child_state, &key_count);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_get_keys_count() Fail(%d)", ret);
				return;
			}
			if (key_count) {
				bool bswitch;
				iotcon_state_get_bool(child_state, "switch", &bswitch);
				DBG("switch : %d", bswitch);
			}
		}
	}
}

static void _on_get_2nd(iotcon_client_h resource, iotcon_representation_h recv_repr,
		iotcon_options_h header_options, int response_result, void *user_data)
{
	_on_get(recv_repr, response_result);
}

static void _on_get_1st(iotcon_client_h resource, iotcon_representation_h recv_repr,
		iotcon_options_h header_options, int response_result, void *user_data)
{
	int ret;
	iotcon_query_h query_params;

	_on_get(recv_repr, response_result);

	ret = iotcon_query_create(&query_params);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_query_create() Fail(%d)", ret);
		return;
	}

	iotcon_query_insert(query_params, "if", "oic.if.b");

	/* send GET request again with BATCH interface */
	iotcon_get(resource, query_params, _on_get_2nd, NULL);

	iotcon_query_destroy(query_params);
}

static int _get_res_type_fn(const char *string, void *user_data)
{
	char *resource_uri_path = user_data;

	DBG("[%s] resource type : %s", resource_uri_path, string);

	return IOTCON_FUNC_CONTINUE;
}

static void _found_resource(iotcon_client_h resource, void *user_data)
{
	int ret;
	char *resource_uri_path;
	char *resource_host;
	char *resource_sid = NULL;
	iotcon_resource_types_h resource_types = NULL;
	int resource_interfaces = 0;

	if (NULL == resource)
		return;

	INFO("===== resource found =====");

	/* get the resource URI */
	ret = iotcon_client_get_uri_path(resource, &resource_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_uri_path() Fail(%d)", ret);
		return;
	}

	/* get the resource server id */
	ret = iotcon_client_get_server_id(resource, &resource_sid);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_server_id() Fail(%d)", ret);
		return;
	}
	DBG("[%s] resource server id : %s", resource_uri_path, resource_sid);

	if (room_resource_sid && TEST_STR_EQUAL == strcmp(room_resource_sid, resource_sid)
			&& TEST_STR_EQUAL == strcmp(room_uri_path, resource_uri_path)) {
		DBG("uri_path \"%s\" already found. skip !", resource_uri_path);
		return;
	}

	room_resource_sid = strdup(resource_sid);

	/* get the resource host address */
	ret = iotcon_client_get_host(resource, &resource_host);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_host() Fail(%d)", ret);
		return;
	}
	DBG("[%s] resource host : %s", resource_uri_path, resource_host);

	/* get the resource interfaces */
	ret = iotcon_client_get_interfaces(resource, &resource_interfaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_interfaces() Fail(%d)", ret);
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
		return;
	}
	iotcon_resource_types_foreach(resource_types, _get_res_type_fn, resource_uri_path);

	if (TEST_STR_EQUAL == strcmp(room_uri_path, resource_uri_path)) {
		/* send GET request */
		iotcon_get(resource, NULL, _on_get_1st, NULL);
	}
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, FALSE);

	/* iotcon open */
	iotcon_open();

	/* find room typed resources */
	iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, "core.room", &_found_resource, NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	/* iotcon close */
	iotcon_close();

	return 0;
}
