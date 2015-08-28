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

#ifndef __TIZEN_NETWORK_IOTCON_DOC_H__
#define __TIZEN_NETWORK_IOTCON_DOC_H__

/**
 * @ingroup CAPI_NETWORK_FRAMEWORK
 * @defgroup CAPI_IOT_CONNECTIVITY_MODULE Iotcon
 *
 * @brief The Iotcon API provides functions for IoT connectivity
 *
 * @section CAPI_IOT_CONNECTIVITY_MODULE_HEADER Required Header
 * \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_MODULE_OVERVIEW Overview
 * The Iotcon API provides to register resources, discover resources and access them via
 * RESTful API.\n\n
 *
 * @subsection CAPI_IOT_CONNECTIVITY_MODULE_RESOURCE Resource
 * A Resrouce is a component in a server that can be viewed and conrolled by another client.\n
 * There are different resource types, for example a temperature sensor, a light controller etc.\n\n
 *
 * @subsection CAPI_IOT_CONNECTIVITY_MODULE_RESOURCE_REGISTRATION Resource registration
 * Registering a resource requires a URI and handler to process requests.\n
 * The URI path should be rooted. Iotcon will construct the fully qualified URI by adding
 * the URI authority to the provided URI path.\n
 * For example, given a service running on port 54321 in a device at IP address 192.168.1.1,
 * if the application registers a resource with a URI path "/light/1",\n
 * the resulting fully qualified URI is "oic://192.168.1.1:54321/light/1",
 * which uniquely identifies the resource's location.\n
 * Note : Only one resource can be registered at a given URI.
 *
 * Example :
 * @code
#include <iotcon.h>
...
static void _request_handler(iotcon_request_h request, void *user_data)
{
	int ret;
	int types;
	iotcon_response_h response;
	iotcon_repr_h resp_repr;

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret)
		return;

	if (IOTCON_REQUEST_GET & types) {
		response = iotcon_response_new(request);
		if (NULL == response)
			return;

		resp_repr = iotcon_repr_new();
		if (NULL == resp_repr) {
			iotcon_response_free(response);
			return;
		}

		ret = iotcon_repr_set_uri_path(resp_repr, "core.door");
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_repr_free(resp_repr);
			iotcon_response_free(response);
			return;
		}

		ret = iotcon_repr_set_bool(resp_repr, "opened", true);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_repr_free(resp_repr);
			iotcon_response_free(response);
			return;
		}

		ret = iotcon_response_set(response, IOTCON_RESPONSE_RESULT, IOTCON_RESPONSE_RESULT_OK);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_repr_free(resp_repr);
			iotcon_response_free(response);
			return;
		}

		ret = iotcon_response_set(response, IOTCON_RESPONSE_REPRESENTATION, resp_repr);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_repr_free(resp_repr);
			iotcon_response_free(response);
			return;
		}

		ret = iotcon_response_send(response);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_repr_free(resp_repr);
			iotcon_response_free(response);
			return;
		}

		iotcon_repr_free(resp_repr);
		iotcon_response_free(response);
	}
}
...
{
	int ret;
	int interfaces = IOTCON_INTERFACE_DEFAULT;
	int properties = (IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE);
	const char *uri_path = "/a/door1";
	const char *type = "core.door";

	iotcon_resource_types_h resource_types = iotcon_resource_types_new();
	if (NULL == resource_types) {
		return;
	}

	ret = iotcon_resource_types_insert(resource_types, type);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(resource_types);
		return;
	}

	iotcon_resource_h handle = iotcon_register_resource(uri_path, resource_types,
			interfaces, properties, _request_handler, NULL);
	if (NULL == handle) {
		iotcon_resource_types_free(resource_types);
		return;
	}

	iotcon_resource_types_free(resource_types);
}
 * @endcode
 *
 * @subsection CAPI_IOT_CONNECTIVITY_MODULE_FINDING_RESOURCE Finding a resource.
 * This operation returns all resources of given type on the network service.\n
 * This operation is sent via multicast to all services.\n
 * If you specify filter resource type in the query, only exact matches will be responded.
 *
 * Example :
 * @code
#include <iotcon.h>
static void _on_get(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, void *user_data)
{
	// handle get from response
}
...
static void _found_resource(iotcon_client_h resource, void *user_data)
{
	int ret;
	int resource_interfaces;
	char *resource_uri_path;
	char *resource_host;
	char *resource_sid;
	iotcon_query_h query;
	iotcon_resource_types_h resource_types;

	ret = iotcon_client_get_uri_path(resource, &resource_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_client_get_server_id(resource, &resource_sid);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_client_get_host(resource, &resource_host);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_client_get_interfaces(resource, &resource_interfaces);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_client_get_types(resource, &resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	query = iotcon_query_new();
	iotcon_query_insert(query, "key", "value");

	iotcon_get(resource, query, &_on_get, NULL);
	iotcon_query_free(query);
}
...
{
	int ret;
	const char *type = "core.door";

	ret = iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, type, &_found_resource, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}
}
 * @endcode
 *
 * @subsection CAPI_IOT_CONNECTIVITY_MODULE_OBSERVING_RESOURCE Observing resource
 * This operation fetches and registers as an observer for the value of simple specific resource.\n
 * An observable resource can handle any number of observers.\n
 * If the server responds with a success code, the registration is considered successful.\n
 * Notifications from the server to the client may be confirmable or non-confirmable.\n
 * If the client returns a RST message, the observation registration should be dropped immediately.\n
 * If the client fails to acknowledge a number of confirmable requests,
 * the server should assume that the client has abandoned the observation and drop the registration.\n
 * If the observed resource is removed, the server sends a NOTFOUND status to all observers.\n
 * If an observed resource fails to notify a client before the max-age of a resource value update,
 * the client should attempt to re-register the observation.\n
 *
 * Example : Server side
 * @code
#include <iotcon.h>
...
static iotcon_resource_h door_handle;
static iotcon_observers_h observers;
...
static void _request_handler(iotcon_request_h request, void *user_data)
{
	int ret;
	int types;
	int observer_id;
	iotcon_observe_action_e observer_action;

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	if (IOTCON_REQUEST_OBSERVE & types) {
		ret = iotcon_request_get_observer_action(request, &observer_action);
		if (IOTCON_ERROR_NONE != ret) {
			return;
		}

		if (IOTCON_OBSERVE_REGISTER == observer_action) {
			ret = iotcon_request_get_observer_id(request, &observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				return;
			}
			observers = iotcon_observers_append(observers, observer_id);
		} else if (IOTCON_OBSERVE_DEREGISTER == observer_action) {
			ret = iotcon_request_get_observer_id(request, &observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				return;
			}
			observers = iotcon_observers_remove(observers, observer_id);
		}
	}
}
...
{
	int ret;
	int interfaces = IOTCON_INTERFACE_DEFAULT;
	int properties = (IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE);
	const char *uri_path = "/a/door1";
	const char *type = "core.door";

	iotcon_resource_types_h resource_types = iotcon_resource_types_new();
	if (NULL == resource_types) {
		return;
	}

	ret = iotcon_resource_types_insert(resource_types, type);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(resource_types);
		return;
	}

	iotcon_resource_h door_handle = iotcon_register_resource(uri_path, resource_types,
			interfaces, properties, _request_handler, NULL);
	if (NULL == door_handle) {
		iotcon_resource_types_free(resource_types);
		return;
	}

	iotcon_resource_types_free(resource_types);
}
...
{
	iotcon_repr_h repr = iotcon_repr_new();
	iotcon_notimsg_h msg = iotcon_notimsg_new(repr, IOTCON_INTERFACE_DEFAULT);

	ret = iotcon_notify_list_of_observers(door_handle, msg, observers);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_notimsg_free(msg);
		iotcon_repr_free(repr);
		return;
	}

	iotcon_notimsg_free(msg);
	iotcon_repr_free(repr);
}
 * @endcode
 *
 * Example : Client side
 * @code
#include <iotcon.h>
...
static iotcon_client_h door_resource;
...
static void _on_observe(iotcon_options_h header_options, iotcon_repr_h recv_repr,
		int response_result, int sequence_number, void *user_data)
{
}
...
{
	int ret;
	ret = iotcon_observer_start(door_resource, IOTCON_OBSERVE_ALL, NULL, &_on_observe, NULL);

	if (IOTCON_ERROR_NONE != ret)
		return;
}
 * @endcode
 *
 * @subsection CAPI_IOT_CONNECTIVITY_MODULE_SERVER_CLIENT_OPERATIONS Server / client operations
 * Server operations should be called in server mode.\n
 * Client operations should be called in client mode.\n
 * Both mode can call both operations.
 * <div><table class="doxtable">
 *     <tr>
 *         <td><b>Server</b></td>
 *         <td><b>Client</b></td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_register_resource</td>
 *         <td>iotcon_get_device_info</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_unregister_resource</td>
 *         <td>iotcon_subscribe_presence</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_bind_interface</td>
 *         <td>iotcon_unsubscribe_presence</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_bind_type</td>
 *         <td>iotcon_find_resource</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_bind_request_handler</td>
 *         <td>iotcon_observer_start</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_bind_resource</td>
 *         <td>iotcon_observer_stop</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_unbind_resource</td>
 *         <td>iotcon_get</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_register_device_info</td>
 *         <td>iotcon_put</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_start_presence</td>
 *         <td>iotcon_post</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_stop_presence</td>
 *         <td>iotcon_delete</td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_response_send</td>
 *         <td></td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_notify_list_of_observers</td>
 *         <td></td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_notify_all</td>
 *         <td></td>
 *     </tr>
 *     <tr>
 *         <td>iotcon_register_device_info</td>
 *         <td></td>
 *     </tr>
 * </table></div>
 *
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE Iotcon Representation
 *
 * @section CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE_HEADER Required Header
 * \#include <iotcon-representation.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE_OVERVIEW Overview
 * The Iotcon Representation API provides data type of representation handling.\n
 * A representation is a payload of a request or a response.\n
 * It has uri_path, interface, list of resource types and its attributes.\n
 * Attributes have capabilties to store and retrieve integer, boolean, double, string, list, null,
 * representation.\n
 * A list is a container that includes number of datas of same type.\n
 * It has capabilties to store and retrieve integer, boolean, double, string, list, null, representation.
 *
 *@code
#include <iotcon.h>
...
{
	int ret;
	iotcon_repr_h repr;
	iotcon_resource_types_h types;
	iotcon_list_h bright_step_list;

	repr = iotcon_repr_new();
	if (NULL == repr)
		return;

	ret = iotcon_repr_set_uri_path(repr, "/a/light");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_repr_free(repr);
		return;
	}

	types = iotcon_resource_types_new();
	if (NULL == types) {
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_resource_types_insert(types, "core.light");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_resource_types(repr, types);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_resource_interfaces(repr, IOTCON_INTERFACE_LINK);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_str(repr, "type", "lamp");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_str(repr, "where", "desk");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_double(repr, "default_bright", 200.0);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_str(repr, "unit", "lux");
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_bool(repr, "bright_step", true);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	bright_step_list = iotcon_list_new(IOTCON_TYPE_DOUBLE);
	if (NULL == bright_step_list) {
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_list_insert_double(bright_step_list, 100.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_free(bright_step_list);
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_list_insert_double(bright_step_list, 200.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_free(bright_step_list);
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_list_insert_double(bright_step_list, 300.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_free(bright_step_list);
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_list_insert_double(bright_step_list, 400.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_free(bright_step_list);
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_list_insert_double(bright_step_list, 500.0, -1);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_free(bright_step_list);
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	ret = iotcon_repr_set_list(repr, "bright_step_list", bright_step_list);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_list_free(bright_step_list);
		iotcon_resource_types_free(types);
		iotcon_repr_free(repr);
		return;
	}

	iotcon_list_free(bright_step_list);
	iotcon_resource_types_free(types);
	iotcon_repr_free(repr);
}
 * @endcode
 */

#endif /* __TIZEN_NETWORK_IOTCON_DOC_H__ */
