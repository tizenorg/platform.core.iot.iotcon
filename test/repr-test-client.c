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
#include "test-log.h"

#define CRUD_MAX_BUFFER_SIZE (256)

const char* const room_uri = "/a/room";

iotcon_client_h room_resource = NULL;

void _get_int_list_fn(int index, const int value, void *user_data)
{
	DBG("%d°C", value);
}

static void _on_get(iotcon_repr_h recv_repr, int response_result)
{
	int i, children_count;
	iotcon_repr_h child_repr;
	iotcon_list_h list;
	iotcon_str_list_s *key_list = NULL;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result, "_on_get_1st Response error(%d)", response_result);
	INFO("GET request was successful");

	DBG("[ parent representation ]");
	DBG("uri : %s", iotcon_repr_get_uri(recv_repr));
	key_list = iotcon_repr_get_key_list(recv_repr);
	if (key_list) {
		DBG("name : %s", iotcon_repr_get_str(recv_repr, "name"));

		list = iotcon_repr_get_list(recv_repr, "today_temp");

		DBG("today's temperature :");
		iotcon_list_foreach_int(list, _get_int_list_fn, NULL);
		iotcon_str_list_free(key_list);
	}

	children_count = iotcon_repr_get_children_count(recv_repr);

	for (i = 0; i < children_count; i++) {
		DBG("[ child representation ]");
		const char *uri;

		child_repr = iotcon_repr_get_nth_child(recv_repr, i);
		uri = iotcon_repr_get_uri(child_repr);
		DBG("uri : %s", uri);

		if (!strcmp("/a/light", uri)) {
			key_list = iotcon_repr_get_key_list(child_repr);
			if (key_list) {
				DBG("brightness : %d", iotcon_repr_get_int(child_repr, "brightness"));
				iotcon_str_list_free(key_list);
			}
		}
		else if (!strcmp("/a/switch", uri)) {
			key_list = iotcon_repr_get_key_list(child_repr);
			if (key_list) {
				DBG("switch : %d", iotcon_repr_get_bool(child_repr, "switch"));
				iotcon_str_list_free(key_list);
			}
		}
	}
}

static void _on_get_2nd(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, void *user_data)
{
	_on_get(recv_repr, response_result);
}

static void _on_get_1st(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, void *user_data)
{
	iotcon_query_h query_params;

	_on_get(recv_repr, response_result);

	query_params = iotcon_query_new();
	iotcon_query_insert(query_params, "if", "oc.mi.b");

	/* send GET request again with BATCH interface */
	iotcon_get(room_resource, query_params, _on_get_2nd, NULL);

	iotcon_query_free(query_params);
}

static void _get_res_type_fn(const char *string, void *user_data)
{
	char *resource_uri = user_data;

	DBG("[%s] resource type : %s", resource_uri, string);
}

static void _found_resource(iotcon_client_h resource, void *user_data)
{
	const char *resource_uri;
	const char *resource_host;
	iotcon_str_list_s *resource_types = NULL;
	int resource_interfaces = 0;

	if (resource) {
		INFO("===== resource found =====");

		/* get the resource URI */
		resource_uri = iotcon_client_get_uri(resource);
		if (NULL == resource_uri) {
			ERR("uri is NULL");
			return;
		}

		/* get the resource host address */
		resource_host = iotcon_client_get_host(resource);
		DBG("[%s] resource host : %s", resource_uri, resource_host);

		/* get the resource interfaces */
		resource_interfaces = iotcon_client_get_interfaces(resource);

		if (IOTCON_INTERFACE_DEFAULT & resource_interfaces)
			DBG("[%s] resource interface : DEFAULT_INTERFACE", resource_uri);
		if (IOTCON_INTERFACE_LINK & resource_interfaces)
			DBG("[%s] resource interface : LINK_INTERFACE", resource_uri);
		if (IOTCON_INTERFACE_BATCH & resource_interfaces)
			DBG("[%s] resource interface : BATCH_INTERFACE", resource_uri);
		if (IOTCON_INTERFACE_GROUP & resource_interfaces)
			DBG("[%s] resource interface : GROUP_INTERFACE", resource_uri);

		/* get the resource types */
		resource_types = iotcon_client_get_types(resource);
		iotcon_str_list_foreach(resource_types, _get_res_type_fn, (void *)resource_uri);

		if (!strcmp(room_uri, resource_uri)) {
			iotcon_query_h query_params;
			/* copy resource to use elsewhere */
			room_resource = iotcon_client_clone(resource);

			query_params = iotcon_query_new();
			/* send GET request */
			iotcon_get(resource, query_params, _on_get_1st, NULL);

			iotcon_query_free(query_params);
		}
	}
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, FALSE);

	/* initialize address and port */
	iotcon_initialize(IOTCON_ALL_INTERFACES, IOTCON_RANDOM_PORT);

	/* find room typed resources */
	iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, "core.room", &_found_resource, NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	return 0;
}
