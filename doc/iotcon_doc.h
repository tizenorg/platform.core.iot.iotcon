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
 * @addtogroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @{
 * @subsection CAPI_IOT_CONNECTIVITY_MODULE_RESOURCE Resource
 * A Resource is a component in a server that can be viewed and conrolled by another client.\n
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
	iotcon_representation_h resp_repr;

	ret = iotcon_request_get_types(request, &types);
	if (IOTCON_ERROR_NONE != ret)
		return;

	if (IOTCON_REQUEST_GET & types) {
		ret = iotcon_response_create(request, &response);
		if (IOTCON_ERROR_NONE != ret)
			return;

		ret = iotcon_representation_create(&resp_repr);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_response_destroy(response);
			return;
		}

		ret = iotcon_representation_set_uri_path(resp_repr, "core.door");
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_representation_destroy(resp_repr);
			iotcon_response_destroy(response);
			return;
		}

		ret = iotcon_state_set_bool(resp_repr, "opened", true);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_representation_destroy(resp_repr);
			iotcon_response_destroy(response);
			return;
		}

		ret = iotcon_response_set_result(response, IOTCON_RESPONSE_RESULT_OK);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_representation_destroy(resp_repr);
			iotcon_response_destroy(response);
			return;
		}


		ret = iotcon_response_set_representation(response, resp_repr);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_representation_destroy(resp_repr);
			iotcon_response_destroy(response);
			return;
		}

		ret = iotcon_response_send(response);
		if (IOTCON_ERROR_NONE != ret) {
			iotcon_representation_destroy(resp_repr);
			iotcon_response_destroy(response);
			return;
		}

		iotcon_representation_destroy(resp_repr);
		iotcon_response_destroy(response);
	}
}
...
{
	int ret;
	int interfaces = IOTCON_INTERFACE_DEFAULT;
	int properties = (IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE);
	const char *uri_path = "/a/door1";
	const char *type = "core.door";
	iotcon_resource_types_h resource_types;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_resource_types_add(resource_types, type);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	iotcon_resource_h handle = iotcon_resource_create(uri_path, resource_types,
			interfaces, properties, _request_handler, NULL);
	if (NULL == handle) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	iotcon_resource_types_destroy(resource_types);
}
 * @endcode
 * @}
 *
 * @addtogroup CAPI_IOT_CONNECTIVITY_CLIENT_MODULE
 * @{
 * @subsection CAPI_IOT_CONNECTIVITY_MODULE_FINDING_RESOURCE Finding a resource.
 * This operation returns all resources of given type on the network service.\n
 * This operation is sent via multicast to all services.\n
 * If you specify filter resource type in the query, only exact matches will be responded.
 *
 * Example :
 * @code
#include <iotcon.h>
static void _on_get(iotcon_options_h header_options, iotcon_representation_h recv_repr,
		int response_result, void *user_data)
{
	// handle get from response
}
...
static void _found_resource(iotcon_remote_resource_h resource, void *user_data)
{
	int ret;
	int resource_interfaces;
	char *resource_uri_path;
	char *resource_host;
	char *device_id;
	iotcon_query_h query;
	iotcon_resource_types_h resource_types;

	ret = iotcon_remote_resource_get_uri_path(resource, &resource_uri_path);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_remote_resource_get_device_id(resource, &device_id);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_remote_resource_get_host_address(resource, &resource_host);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_remote_resource_get_interfaces(resource, &resource_interfaces);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_remote_resource_get_types(resource, &resource_types);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_query_create(&query);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	iotcon_query_add(query, "key", "value");

	iotcon_remote_resource_get(resource, query, &_on_get, NULL);
	iotcon_query_destroy(query);
}
...
{
	int ret;
	const char *type = "core.door";

	ret = iotcon_find_resource(IOTCON_MULTICAST_ADDRESS, IOTCON_CONNECTIVITY_IPV4, type,
			&_found_resource, NULL);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}
}
 * @endcode
 * @}
 *
 * @addtogroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @{
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
			ret = iotcon_observers_add(observers, observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				return;
			}
		} else if (IOTCON_OBSERVE_DEREGISTER == observer_action) {
			ret = iotcon_request_get_observer_id(request, &observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				return;
			}
			ret = iotcon_observers_remove(observers, observer_id);
			if (IOTCON_ERROR_NONE != ret) {
				return;
			}
		}
	}
}
...
{
	int ret;
	int interfaces = IOTCON_INTERFACE_DEFAULT;
	int properties = (IOTCON_DISCOVERABLE | IOTCON_OBSERVABLE);
	iotcon_resource_h door_handle;
	const char *uri_path = "/a/door1";
	const char *type = "core.door";
	iotcon_resource_types_h resource_types;

	ret = iotcon_resource_types_create(&resource_types);
	if (IOTCON_ERROR_NONE == ret) {
		return;
	}

	ret = iotcon_resource_types_add(resource_types, type);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	ret = iotcon_resource_create(uri_path, resource_types,
			interfaces, properties, _request_handler, NULL, door_handle);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_resource_types_destroy(resource_types);
		return;
	}

	iotcon_resource_types_destroy(resource_types);
}
...
{
	int ret;
	iotcon_representation_h repr;
	iotcon_notimsg_h msg;

	ret = iotcon_representation_create(&resp_repr);
	if (IOTCON_ERROR_NONE != ret) {
		return;
	}

	ret = iotcon_notimsg_create(resp_repr, IOTCON_INTERFACE_DEFAULT, &msg);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_representation_destroy(resp_repr);
		return;
	}

	ret = iotcon_resource_notify(door_handle, msg, observers);
	if (IOTCON_ERROR_NONE != ret) {
		iotcon_notimsg_destroy(msg);
		iotcon_representation_destroy(resp_repr);
		return;
	}

	iotcon_notimsg_destroy(msg);
	iotcon_representation_destroy(resp_repr);
}
 * @endcode
 *
 * Example : Client side
 * @code
#include <iotcon.h>
...
static iotcon_remote_resource_h door_resource;
...
static void _on_observe(iotcon_options_h header_options, iotcon_representation_h recv_repr,
		int response_result, int sequence_number, void *user_data)
{
}
...
{
	int ret;
	ret = iotcon_remote_resource_start_observing(door_resource, IOTCON_OBSERVE_ALL, NULL,
			&_on_observe, NULL);
	if (IOTCON_ERROR_NONE != ret)
		return;
}
 * @endcode
 * @}
 *
 */

#endif /* __TIZEN_NETWORK_IOTCON_DOC_H__ */
