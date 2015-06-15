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

const char * const door_uri = "/a/door";

iotcon_client_h door_resource = NULL;

void _print_repr_info(iotcon_repr_h repr)
{
	if (0 < iotcon_repr_get_keys_count(repr))
		DBG("rep : \n%s", iotcon_repr_generate_json(repr));
}

static void _on_observe(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, int sequence_number, void *user_data)
{
	INFO("_on_observe");

	static int i = 0;
	i++;

	if (2 == i)
		iotcon_observer_stop(door_resource);
}

static void _on_delete(iotcon_options_h header_options, int response_result,
		void *user_data)
{
	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result
			&& IOTCON_RESPONSE_RESULT_RESOURCE_DELETED != response_result,
			"_on_delete Response error(%d)", response_result);
	INFO("DELETE request was successful");

	/* delete callback operations */

	iotcon_observer_start(door_resource, IOTCON_OBSERVE_ALL, NULL, _on_observe, NULL);
}

static void _on_post(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, void *user_data)
{
	int ret;
	char *created_uri = NULL;
	iotcon_client_h new_door_resource = NULL;
	char *host = NULL;
	iotcon_resource_types_h types = NULL;
	int ifaces = 0;

	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result
			&& IOTCON_RESPONSE_RESULT_RESOURCE_CREATED != response_result,
			"_on_post Response error(%d)", response_result);
	INFO("POST request was successful");

	_print_repr_info(recv_repr);

	iotcon_repr_get_str(recv_repr, "createduri", &created_uri);

	if (NULL == created_uri) {
		ERR("created_uri is NULL");
		return;
	}

	DBG("New resource created : %s", created_uri);

	ret = iotcon_client_get_host(door_resource, &host);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_host() Fail(%d)", ret);
		return;
	}

	ret = iotcon_client_get_types(door_resource, &types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_types() Fail(%d)", ret);
		return;
	}

	ret = iotcon_client_get_interfaces(door_resource, &ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_ifaces() Fail(%d)", ret);
		return;
	}

	new_door_resource = iotcon_client_new(host, created_uri, true, types, ifaces);

	iotcon_delete(new_door_resource, _on_delete, NULL);

	iotcon_client_free(new_door_resource);
}

static void _on_put(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, void *user_data)
{
	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result, "_on_put Response error(%d)",
			response_result);
	INFO("PUT request was successful");

	_print_repr_info(recv_repr);

	iotcon_repr_h send_repr = iotcon_repr_new();

	iotcon_query_h query_params = iotcon_query_new();
	/* send POST request */
	iotcon_post(door_resource, send_repr, query_params, _on_post, NULL);

	iotcon_repr_free(send_repr);
	iotcon_query_free(query_params);

}

static void _on_get(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, void *user_data)
{
	RETM_IF(IOTCON_RESPONSE_RESULT_OK != response_result, "_on_get Response error(%d)",
			response_result);
	INFO("GET request was successful");

	_print_repr_info(recv_repr);

	iotcon_repr_h send_repr = iotcon_repr_new();
	iotcon_repr_set_bool(send_repr, "opened", true);

	iotcon_query_h query_params = iotcon_query_new();
	/* send PUT request */
	iotcon_put(door_resource, send_repr, query_params, _on_put, NULL);

	iotcon_repr_free(send_repr);
	iotcon_query_free(query_params);
}

static int _get_res_type_fn(const char *string, void *user_data)
{
	char *resource_uri = user_data;

	DBG("[%s] resource type : %s", resource_uri, string);

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

static void _found_resource(iotcon_client_h resource, void *user_data)
{
	int ret;
	char *resource_uri = NULL;
	char *resource_host = NULL;
	iotcon_resource_types_h resource_types = NULL;
	int resource_interfaces = 0;

	if (NULL == resource)
		return;

	INFO("===== resource found =====");

	/* get the resource URI */
	ret = iotcon_client_get_uri(resource, &resource_uri);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_uri() Fail(%d)", ret);
		return;
	}

	/* get the resource host address */
	ret = iotcon_client_get_host(resource, &resource_host);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_host() Fail(%d)", ret);
		return;
	}
	DBG("[%s] resource host : %s", resource_uri, resource_host);

	/* get the resource interfaces */
	ret = iotcon_client_get_interfaces(resource, &resource_interfaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_interfaces() Fail(%d)", ret);
		return;
	}
	if (IOTCON_INTERFACE_DEFAULT & resource_interfaces)
		DBG("[%s] resource interface : DEFAULT_INTERFACE", resource_uri);
	if (IOTCON_INTERFACE_LINK & resource_interfaces)
		DBG("[%s] resource interface : LINK_INTERFACE", resource_uri);
	if (IOTCON_INTERFACE_BATCH & resource_interfaces)
		DBG("[%s] resource interface : BATCH_INTERFACE", resource_uri);
	if (IOTCON_INTERFACE_GROUP & resource_interfaces)
		DBG("[%s] resource interface : GROUP_INTERFACE", resource_uri);

	/* get the resource types */
	ret = iotcon_client_get_types(resource, &resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_client_get_types() Fail(%d)", ret);
		return;
	}

	ret = iotcon_resource_types_foreach(resource_types, _get_res_type_fn,
			resource_uri);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_foreach() Fail(%d)", ret);
		return;
	}

	iotcon_subscribe_presence(resource_host, "core.door", _presence_handler, NULL);

	if (TEST_STR_EQUAL == strcmp(door_uri, resource_uri)) {
		door_resource = iotcon_client_clone(resource);

		iotcon_query_h query = iotcon_query_new();
		iotcon_query_insert(query, "key", "value");

		/* send GET Request */
		iotcon_get(resource, query, _on_get, NULL);
		iotcon_query_free(query);
	}
}

int main(int argc, char **argv)
{
	FN_CALL;
	GMainLoop *loop;
	loop = g_main_loop_new(NULL, FALSE);

	/* initialize address and port */
	iotcon_initialize(IOTCON_ALL_INTERFACES, IOTCON_RANDOM_PORT);

	/* find door typed resources */
	iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, "core.door", &_found_resource, NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	return 0;
}
