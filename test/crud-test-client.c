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
#include <iotcon.h>
#include "test-log.h"

#define CRUD_MAX_BUFFER_SIZE (256)

const char* const door_uri = "/a/door";

iotcon_resource_s door_resource = {0};

char* _alloc_str_from_glist(GList *list)
{
	int i;
	char buf[CRUD_MAX_BUFFER_SIZE] = {0};
	char *ret_str = NULL;

	for (i = 0; i < g_list_length(list); i++) {
		char *str = g_list_nth_data(list, i);
		strncat(buf, str, strlen(str));
	}

	ret_str = strdup(buf);
	return ret_str;
}

void _print_repr_info(iotcon_repr_h repr)
{
	if (0 < iotcon_repr_get_keys_count(repr))
		DBG("rep : \n%s", iotcon_repr_generate_json(repr));
}

static void _on_delete(const iotcon_options_h header_options, const int e_code,
		void *user_data)
{
	RETM_IF(IOTCON_EH_OK != e_code && IOTCON_EH_RESOURCE_DELETED != e_code,
			"_on_delete Response error(%d)", e_code);
	INFO("DELETE request was successful");

	/* delete callback operations */
}

static void _on_post(const iotcon_options_h header_options, iotcon_repr_h recv_repr,
		const int e_code, void *user_data)
{
	char *created_uri = NULL;
	iotcon_resource_s new_door_resource = {0};

	RETM_IF(IOTCON_EH_OK != e_code, "_on_post Response error(%d)", e_code);
	INFO("POST request was successful");

	_print_repr_info(recv_repr);

	created_uri = iotcon_repr_get_str(recv_repr, "createduri");
	if (created_uri) {
		DBG("New resource created : %s", created_uri);

		new_door_resource = iotcon_construct_resource_object(door_resource.resource_host,
				created_uri,
				true, door_resource.resource_types, door_resource.resource_interfaces);

		iotcon_delete_resource(new_door_resource, _on_delete, NULL);
	}

}

static void _on_put(const iotcon_options_h header_options, iotcon_repr_h recv_repr,
		const int e_code, void *user_data)
{
	RETM_IF(IOTCON_EH_OK != e_code, "_on_put Response error(%d)", e_code);
	INFO("PUT request was successful");

	_print_repr_info(recv_repr);

	iotcon_repr_h send_repr = iotcon_repr_new();

	iotcon_query query_params = iotcon_query_new();
	/* send POST request */
	iotcon_post(door_resource, send_repr, query_params, _on_post, NULL);

	iotcon_repr_free(send_repr);
	iotcon_query_free(query_params);

}

static void _on_get(const iotcon_options_h header_options, iotcon_repr_h recv_repr,
		const int e_code, void *user_data)
{
	RETM_IF(IOTCON_EH_OK != e_code, "_on_get Response error(%d)", e_code);
	INFO("GET request was successful");

	_print_repr_info(recv_repr);

	iotcon_repr_h send_repr = iotcon_repr_new();
	iotcon_repr_set_bool(send_repr, "opened", true);

	iotcon_query query_params = iotcon_query_new();
	/* send PUT request */
	iotcon_put(door_resource, send_repr, query_params, _on_put, NULL);

	iotcon_repr_free(send_repr);
	iotcon_query_free(query_params);

}

static void _found_resource(iotcon_resource_s *resource, void *user_data)
{
	char *resource_uri = NULL;
	char *resource_host = NULL;
	iotcon_resource_types resource_types = NULL;
	iotcon_resource_interfaces resource_interfaces = NULL;

	if (resource) {
		char *interfaces_str = NULL;
		char *res_types_str = NULL;

		INFO("===== resource found =====");

		/* get the resource URI */
		resource_uri = iotcon_resource_get_uri(*resource);
		if (NULL == resource_uri) {
			ERR("uri is NULL");
			return;
		}

		/* get the resource host address */
		resource_host = iotcon_resource_get_host(*resource);
		DBG("[%s] resource host : %s", resource_uri, resource_host);

		/* get the resource interfaces */
		resource_interfaces = iotcon_resource_get_interfaces(*resource);
		if (resource_interfaces) {
			interfaces_str = _alloc_str_from_glist(resource_interfaces);
			DBG("[%s] resource interfaces : %s", resource_uri, interfaces_str);
			free(interfaces_str);
		}

		/* get the resource types */
		resource_types = iotcon_resource_get_types(*resource);
		if (resource_types) {
			res_types_str = _alloc_str_from_glist(resource_types);
			DBG("[%s] resource types : %s", resource_uri, res_types_str);
			free(res_types_str);
		}

		if (!strcmp(door_uri, resource_uri)) {
			/* copy resource to use elsewhere */
			door_resource = iotcon_copy_resource(*resource);

			iotcon_query query_params = iotcon_query_new();
			/* send GET request */
			iotcon_get(*resource, query_params, _on_get, NULL);

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
	iotcon_initialize("0.0.0.0", 0);

	/* find door typed resources */
	iotcon_find_resource("", "coap://224.0.1.187/oc/core?rt=core.door", &_found_resource,
	NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	return 0;
}
