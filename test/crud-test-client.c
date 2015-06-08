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

void _get_res_type_fn(const char *res_type, void *user_data)
{
	DBG("resource type : %s", res_type);
}

void _get_res_interface_fn(const char *res_if, void *user_data)
{
	DBG("resource interface : %s", res_if);
}

void _print_repr_info(iotcon_repr_h repr)
{
	const char *uri = iotcon_repr_get_uri(repr);
	if (uri)
		DBG("uri : %s", uri);

	if (0 < iotcon_repr_get_keys_count(repr)) {
		DBG("rep :");
		iotcon_repr_print(repr);
	}

	iotcon_repr_get_resource_types(repr, _get_res_type_fn, NULL);
	iotcon_repr_get_resource_interfaces(repr, _get_res_interface_fn, NULL);

}

static void _on_put(const iotcon_header_options header_options, iotcon_repr_h repr,
		const int e_code, void *user_data)
{
	FN_CALL;

	if (e_code == IOTCON_ERR_NONE) {
		INFO("PUT request was successful");

		DBG("[%s] parent", iotcon_repr_get_uri(repr));
		_print_repr_info(repr);

		GList *children = iotcon_repr_get_children(repr);
		if (children) {
			int child_count = iotcon_repr_get_children_count(repr);
			int child_index = 0;
			for (child_index = 0; child_index < child_count; child_index++) {
				DBG("[%s] %dth child", child_index + 1, iotcon_repr_get_uri(repr));
				_print_repr_info(iotcon_repr_get_nth_child(repr, child_index));
			}
		}
	}
	else {
		ERR("onPUT Response error(%d)", e_code);
	}
	/* set the flag for receiving response succesfully */
}

void _on_get(const iotcon_header_options header_options, iotcon_repr_h repr,
		const int e_code, void *user_data)
{
	FN_CALL;

	if (e_code == IOTCON_ERR_NONE) {
		INFO("GET request was successful");

		DBG("[%s] parent", iotcon_repr_get_uri(repr));
		_print_repr_info(repr);

		GList *children = iotcon_repr_get_children(repr);
		if (children) {
			int child_count = iotcon_repr_get_children_count(repr);
			int child_index = 0;
			for (child_index = 0; child_index < child_count; child_index++) {
				DBG("[%s] %dth child", child_index + 1, iotcon_repr_get_uri(repr));
				_print_repr_info(iotcon_repr_get_nth_child(repr, child_index));
			}
		}

		iotcon_repr_h repr = iotcon_repr_new();
		iotcon_repr_set_bool(repr, "opened", true);

		iotcon_query_parameters query_params = iotcon_new_query_params();
		iotcon_put(door_resource, repr, query_params, _on_put, NULL);

	}
	else {
		ERR("onGET Response error(%d)", e_code);
	}
	/* set the flag for receiving response succesfully */
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

		/* Get the resource URI */
		resource_uri = iotcon_get_resource_uri(*resource);
		if (NULL == resource_uri) {
			ERR("uri is NULL");
			return;
		}
		/* Get the resource host address */
		resource_host = iotcon_get_resource_host(*resource);
		DBG("[%s] resource host : %s", resource_uri, resource_host);

		/* Get the resource interfaces */
		resource_interfaces = iotcon_get_resource_interfaces(*resource);
		if (resource_interfaces) {
			interfaces_str = _alloc_str_from_glist(resource_interfaces);
			DBG("[%s] resource interfaces : %s", resource_uri, interfaces_str);
			free(interfaces_str);
		}

		/* Get the resource types */
		resource_types = iotcon_get_resource_types(*resource);
		if (resource_types) {
			res_types_str = _alloc_str_from_glist(resource_types);
			DBG("[%s] resource types : %s", resource_uri, res_types_str);
			free(res_types_str);
		}

		if (!strcmp(door_uri, resource_uri)) {
			door_resource = iotcon_copy_resource(*resource);

			iotcon_query_parameters query_params = iotcon_new_query_params();

			/* send GET Request */
			iotcon_get(*resource, query_params, _on_get, NULL);

			iotcon_delete_query_params(query_params);
		}
	}
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, FALSE);

	iotcon_initialize("0.0.0.0", 0);

	iotcon_find_resource("", "coap://224.0.1.187/oc/core?rt=core.door", &_found_resource, NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	return 0;
}
