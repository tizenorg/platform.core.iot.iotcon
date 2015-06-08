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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>
#include <glib-object.h>

#include "iotcon.h"

#include "ic-common.h"
#include "ic-utils.h"
#include "ic-struct.h"
#include "ic-ioty.h"

/**
 * @brief global context
 */
static ic_ctx_s ic_ctx;
static gboolean ic_is_init = FALSE;

static const char *IC_INTERFACE_DEFAULT = "oc.mi.def";
static const char *IC_INTERFACE_LINK = "oc.mi.ll";
static const char *IC_INTERFACE_BATCH = "oc.mi.b";
static const char *IC_INTERFACE_GROUP = "oc.mo.grp";

ic_ctx_s* ic_get_ctx()
{
	return &ic_ctx;
}


static void _delete_resource_value(gpointer data)
{
	resource_handler_s *value = data;

	RET_IF(NULL == data);

	free(value->if_name);
	free(value->rt_name);
	free(value->uri_name);
	value->rest_api_cb = NULL;
}

gboolean iotcon_is_init(void)
{
	return ic_is_init;
}

static void _set_iotcon_init(gboolean tag)
{
	ic_is_init = tag;
}

API void iotcon_initialize(const char *addr, unsigned short port)
{
	FN_CALL;

	RETM_IF(TRUE == iotcon_is_init(), "already initialized");

	ic_iotivity_config(addr, port);
	ic_ctx.entity_cb_hash = g_hash_table_new_full(g_int_hash, g_str_equal, NULL,
			_delete_resource_value);
#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif
	_set_iotcon_init(TRUE);
}

API void iotcon_deinitialize()
{
	FN_CALL;

	RETM_IF(FALSE == iotcon_is_init(), "Not initialized");

	g_hash_table_destroy(ic_ctx.entity_cb_hash);
	g_list_free(ic_ctx.found_device_cb_lst);
	_set_iotcon_init(FALSE);
}

API iotcon_response_h iotcon_create_response(iotcon_request_h req_h,
		iotcon_resource_h res_h)
{
	FN_CALL;

	iotcon_response_h resp = calloc(1, sizeof(struct ic_res_response_s));
	if (NULL == resp) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	resp->request_handle = req_h;
	resp->resource_handle = res_h;

	return resp;
}


API int iotcon_response_set(iotcon_response_h resp, iotcon_response_property_e prop,
		...)
{
	int value;
	va_list args;

	va_start(args, prop);

	switch (prop) {
	case IOTCON_RESP_REPRESENTATION:
		resp->repr = va_arg(args, iotcon_repr_h);
		break;
	case IOTCON_RESP_RESULT:
		value = va_arg(args, int);
		if (value < IOTCON_EH_OK || IOTCON_EH_MAX <= value) {
			ERR("Invalid value");
			return IOTCON_ERR_PARAM;
		}
		resp->result = value;
		break;
	case IOTCON_RESP_ERR_CODE:
		resp->error_code = va_arg(args, int);
		break;
	case IOTCON_RESP_RES_URI:
		resp->new_resource_uri = va_arg(args, char*);
		break;
	case IOTCON_RESP_NONE:
	default:
		break;
	}

	va_end(args);

	return IOTCON_ERR_NONE;
}

API void iotcon_delete_response(iotcon_response_h resp)
{
	FN_CALL;
	struct ic_res_response_s *data = resp;

	RET_IF(NULL == resp);

	free(data->new_resource_uri);
	iotcon_delete_header_options(data->header_options);
	iotcon_repr_free(resp->repr);
	free(data);
}

API iotcon_repr_h iotcon_request_get_representation(const iotcon_request_s *request)
{
	RETV_IF(NULL == request, NULL);
	return request->repr;
}

API iotcon_resource_h iotcon_register_resource(const char *iot_uri,
		const char *iot_rt,
		iotcon_interface_e iot_if,
		iotcon_resource_property_e iot_rt_type,
		iotcon_rest_api_handle_cb entity_handler_cb)
{
	FN_CALL;

	resource_handler_s *res;
	iotcon_resource_h ret_handle;

	RETV_IF(NULL == iot_uri, NULL);
	RETV_IF(NULL == iot_rt, NULL);
	RETV_IF(NULL == entity_handler_cb, NULL);

	res = calloc(1, sizeof(resource_handler_s));
	if (NULL == res) {
		ERR("calloc Fail(%d)", errno);
		return NULL;
	}

	switch (iot_if) {
	case IOTCON_INTERFACE_GROUP:
		res->if_name = ic_utils_strdup(IC_INTERFACE_GROUP);
		break;
	case IOTCON_INTERFACE_BATCH:
		res->if_name = ic_utils_strdup(IC_INTERFACE_BATCH);
		break;
	case IOTCON_INTERFACE_LINK:
		res->if_name = ic_utils_strdup(IC_INTERFACE_LINK);
		break;
	case IOTCON_INTERFACE_DEFAULT:
	default:
		res->if_name = ic_utils_strdup(IC_INTERFACE_DEFAULT);
		break;
	}

	res->rt_name = ic_utils_strdup(iot_rt);
	if (NULL == res->rt_name) {
		ERR("ic_utils_strdup() Fail(%d)", errno);
		free(res);
		return NULL;
	}

	res->uri_name = ic_utils_strdup(iot_uri);
	if (NULL == res->uri_name) {
		ERR("ic_utils_strdup() Fail(%d)", errno);
		free(res->rt_name);
		free(res);
		return NULL;
	}

	res->rest_api_cb = entity_handler_cb;

	ret_handle = ic_ioty_register_res(iot_uri, iot_rt, iot_if, iot_rt_type);
	if (NULL == ret_handle) {
		ERR("registerResource Fail");
		free(res->rt_name);
		free(res->uri_name);
		free(res);
		return NULL;
	}

	g_hash_table_insert(ic_ctx.entity_cb_hash, ret_handle, res);

	return ret_handle;
}


API int iotcon_unregister_resource(const iotcon_resource_h resource_handle)
{
	FN_CALL;

	int ret;

	ret = ic_ioty_unregister_res(resource_handle);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_bind_type_to_res() Fail(%d)", ret);

	g_hash_table_remove(ic_ctx.entity_cb_hash, resource_handle);

	return ret;
}

API int iot_bind_interface_to_resource(iotcon_resource_h resource_handle,
		const char *interface_type)
{
	FN_CALL;

	int ret;

	ret = ic_ioty_bind_iface_to_res(resource_handle, interface_type);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_bind_type_to_res() Fail(%d)", ret);

	return ret;
}

API int iotcon_bind_type_to_resource(iotcon_resource_h resource_handle,
		const char *resource_type)
{
	FN_CALL;

	int ret;

	ret = ic_ioty_bind_type_to_res(resource_handle, resource_type);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_bind_type_to_res() Fail(%d)", ret);

	return ret;
}

API int iotcon_bind_resource(iotcon_resource_h parent, iotcon_resource_h child)
{
	FN_CALL;

	int ret;

	ret = ic_ioty_bind_res(parent, child);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_bind_res() Fail(%d)", ret);

	return ret;
}


API int iotcon_register_device_info(iotcon_device_info_s *device_info)
{
	FN_CALL;

	int ret;

	ret = ic_ioty_register_device_info(device_info);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_register_device_info() Fail(%d)", ret);

	return ret;
}

API int iotcon_subscribe_device_info(char *host, char *uri,
		iotcon_found_device_info_cb found_cb /*, int QoS*/)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	RETV_IF(NULL == uri, IOTCON_ERR_PARAM);
	RETV_IF(NULL == found_cb, IOTCON_ERR_PARAM);

	/* If we add device_cb to manage app's CB if we don't have any app's cb */
	if (0 == g_list_length(ic_ctx.found_device_cb_lst)) {
		ret = ic_ioty_get_device_info(host, uri);
		if (IOTCON_ERR_NONE != ret) {
			ERR("ic_ioty_get_device_info() Fail(%d)", ret);
			return ret;
		}
	}

	ic_ctx.found_device_cb_lst = g_list_append(ic_ctx.found_device_cb_lst, found_cb);

	return IOTCON_ERR_NONE;
}

API void iotcon_unsubscribe_device_info(char *host, char *uri,
		iotcon_found_device_info_cb found_cb)
{
	FN_CALL;

	GList *node = g_list_first(ic_ctx.found_device_cb_lst);
	while (node) {
		GList *next_node = node->next;

		iotcon_found_device_info_cb found_cb_check = node->data;
		if (found_cb_check == found_cb) {
			ic_ctx.found_device_cb_lst = g_list_remove_link(ic_ctx.found_device_cb_lst,
					node);
			g_list_free_1(node);
		}
		node = next_node;
	}
}

API int iotcon_send_notify_response(iotcon_response_h resp, iotcon_observers observers)
{
	FN_CALL;
	int ret;

	ret = ic_ioty_send_notify(resp, observers);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_send_notify() Fail(%d)", ret);

	return ret;
}

API int iotcon_send_resource_response(iotcon_response_h resp)
{
	FN_CALL;
	int ret;

	ret = ic_ioty_send_res_response_data(resp);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_send_res_response_data() Fail(%d)", ret);

	return ret;
}

API iotcon_presence_h iotcon_subscribe_presence(const char *host_address,
		iotcon_presence_handle_cb presence_handler_cb, void *user_data)
{
	iotcon_presence_h handle;

	handle = ic_ioty_subscribe_presence(host_address, presence_handler_cb, user_data);
	if (NULL == handle)
		ERR("ic_ioty_subscribe_presence() Fail");

	return handle;
}

API int iotcon_unsubscribe_presence(iotcon_presence_h presence_handle)
{
	FN_CALL;

	int ret;

	ret = ic_ioty_unsubscribe_presence(presence_handle);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_unsubscribe_presence() Fail(%d)", ret);

	return ret;
}

API int iotcon_start_presence(const unsigned int time_to_live)
{
	FN_CALL;

	int ret;

	ret = ic_ioty_start_presence(time_to_live);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_start_presence() Fail(%d)", ret);

	return ret;
}

API int iotcon_stop_presence()
{
	FN_CALL;

	int ret;

	ret = ic_ioty_stop_presence();
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_stop_presence() Fail(%d)", ret);

	return ret;
}

API int iotcon_find_resource(const char *host, const char *resource_name,
		iotcon_found_resource_cb found_resource_cb, void *user_data)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	RETV_IF(NULL == host, IOTCON_ERR_PARAM);
	RETV_IF(NULL == resource_name, IOTCON_ERR_PARAM);
	RETV_IF(NULL == found_resource_cb, IOTCON_ERR_PARAM);

	ret = ic_ioty_find_resource(host, resource_name, found_resource_cb, user_data);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_find_resource() Fail(%d)", ret);

	return ret;
}

API iotcon_resource_s iotcon_construct_resource_object(const char *host,
												const char *uri,
												bool is_observable,
												iotcon_resource_types resource_type,
												iotcon_resource_interfaces resource_if)
{
	FN_CALL;

	iotcon_resource_s resource_s = {0};
	GList *node = NULL;

	resource_s.resource_host = ic_utils_strdup(host);
	resource_s.resource_uri = ic_utils_strdup(uri);
	resource_s.is_observable = is_observable;
	resource_s.resource_types = iotcon_new_resource_types();
	resource_s.resource_interfaces = iotcon_new_resource_interfaces();

	for (node = g_list_first(resource_type); NULL != node; node = g_list_next(node)) {
		resource_s.resource_types = g_list_append(resource_s.resource_types,
				ic_utils_strdup(node->data));
	}

	for (node = g_list_first(resource_if); NULL != node; node = g_list_next(node)) {
		resource_s.resource_interfaces = g_list_append(resource_s.resource_interfaces,
				ic_utils_strdup(node->data));
	}

	return resource_s;
}

API void iotcon_destruct_resource_object(iotcon_resource_s *resource)
{
	FN_CALL;

	free(resource->resource_uri);
	free(resource->resource_host);
	iotcon_delete_header_options(resource->header_options);
	iotcon_delete_resource_types(resource->resource_types);
	iotcon_delete_resource_interfaces(resource->resource_interfaces);
}

API iotcon_resource_s iotcon_copy_resource(iotcon_resource_s resource)
{
	FN_CALL;

	iotcon_resource_s resource_s = {0};
	GList *node = NULL;

	resource_s.resource_host = ic_utils_strdup(resource.resource_host);
	resource_s.resource_uri = ic_utils_strdup(resource.resource_uri);
	resource_s.is_observable = resource.is_observable;
	resource_s.resource_types = iotcon_new_resource_types();
	resource_s.resource_interfaces = iotcon_new_resource_interfaces();
	resource_s.observe_handle = resource.observe_handle;

	for (node = g_list_first(resource.resource_types); NULL != node;
			node = g_list_next(node)) {
		resource_s.resource_types = g_list_append(resource_s.resource_types,
				ic_utils_strdup(node->data));
	}

	for (node = g_list_first(resource.resource_interfaces); NULL != node;
			node = g_list_next(node)) {
		resource_s.resource_interfaces = g_list_append(resource_s.resource_interfaces,
				ic_utils_strdup(node->data));
	}

	return resource_s;
}

API int iotcon_get(iotcon_resource_s resource, iotcon_query_parameters query_params,
		iotcon_on_get_cb on_get_cb, void *user_data)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	ret = ic_ioty_get(resource, query_params, on_get_cb, user_data);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_get() Fail(%d)", ret);

	return ret;
}

API int iotcon_put(iotcon_resource_s resource, iotcon_repr_h repr,
		iotcon_query_parameters query_params, iotcon_on_put_cb on_put_cb, void *user_data)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);

	ret = ic_ioty_put(resource, repr, query_params, on_put_cb, user_data);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_put() Fail(%d)", ret);

	return ret;
}

API int iotcon_post(iotcon_resource_s resource,
		iotcon_repr_h repr,
		iotcon_query_parameters query_params,
		iotcon_on_post_cb on_post_cb,
		void *user_data)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);

	ret = ic_ioty_post(resource, repr, query_params, on_post_cb, user_data);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_post() Fail(%d)", ret);

	return ret;
}

API int iotcon_delete_resource(iotcon_resource_s resource,
		iotcon_on_delete_cb on_delete_cb, void *user_data)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	ret = ic_ioty_delete_res(resource, on_delete_cb, user_data);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_delete_res() Fail(%d)", ret);

	return ret;
}

API int iotcon_observe(iotcon_observe_type_e observe_type,
		iotcon_resource_s *resource,
		iotcon_query_parameters query_params,
		iotcon_on_observe_cb on_observe_cb,
		void *user_data)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	ret = ic_ioty_observe(resource, observe_type, query_params, on_observe_cb,
			user_data);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_observe() Fail(%d)", ret);

	return ret;
}

API int iotcon_cancel_observe(iotcon_resource_s resource)
{
	FN_CALL;

	int ret = IOTCON_ERR_NONE;

	ret = ic_ioty_cancel_observe(resource);
	if (IOTCON_ERR_NONE != ret)
		ERR("ic_ioty_cancel_observe() Fail(%d)", ret);

	return ret;
}

API iotcon_resource_types iotcon_new_resource_types()
{
	return NULL;
}

API iotcon_resource_types iotcon_add_resource_types(iotcon_resource_types resource_types,
		const char *resource_type)
{
	FN_CALL;

	resource_types = g_list_append(resource_types, ic_utils_strdup(resource_type));
	return resource_types;
}

API void iotcon_delete_resource_types(iotcon_resource_types resource_types)
{
	FN_CALL;

	g_list_free_full(resource_types, g_free);
}

API iotcon_resource_interfaces iotcon_new_resource_interfaces()
{
	FN_CALL;

	return NULL;
}

API iotcon_resource_interfaces iotcon_add_resource_interfaces(
		iotcon_resource_interfaces resource_interfaces, iotcon_interface_e interface)
{
	FN_CALL;

	char *resource_interface;

	switch (interface) {
	case IOTCON_INTERFACE_GROUP:
		resource_interface = ic_utils_strdup(IC_INTERFACE_GROUP);
		break;
	case IOTCON_INTERFACE_BATCH:
		resource_interface = ic_utils_strdup(IC_INTERFACE_BATCH);
		break;
	case IOTCON_INTERFACE_LINK:
		resource_interface = ic_utils_strdup(IC_INTERFACE_LINK);
		break;
	case IOTCON_INTERFACE_DEFAULT:
		resource_interface = ic_utils_strdup(IC_INTERFACE_DEFAULT);
		break;
	default:
		ERR("Invalid Interface");
		return resource_interfaces;
	}

	resource_interfaces = g_list_append(resource_interfaces, resource_interface);

	return resource_interfaces;
}

API void iotcon_delete_resource_interfaces(iotcon_resource_interfaces resource_interfaces)
{
	FN_CALL;

	g_list_free_full(resource_interfaces, g_free);
}

API iotcon_header_options iotcon_new_header_options()
{
	iotcon_header_options options = NULL;
	options = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
	return options;
}

API iotcon_error_e iotcon_add_header_option(iotcon_header_options options,
		const unsigned short id, const char *data)
{
	FN_CALL;

	RETV_IF(((id < IOTCON_HEADER_OPTIONID_MIN)
				|| (IOTCON_HEADER_OPTIONID_MAX < id)), IOTCON_ERR_PARAM);

	gpointer value = ic_utils_strdup(data);

	g_hash_table_insert(options, GUINT_TO_POINTER(id), value);

	return IOTCON_ERR_NONE;
}

API void iotcon_delete_header_options(iotcon_header_options options)
{
	RET_IF(NULL == options);

	g_hash_table_destroy(options);
}

API iotcon_query_parameters iotcon_new_query_params()
{
	iotcon_query_parameters query_params = NULL;

	query_params = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	return query_params;
}

API void iotcon_add_query_param(iotcon_query_parameters query_params, const char *key,
		const char *value)
{
	g_hash_table_insert(query_params, ic_utils_strdup(key), ic_utils_strdup(value));
}

API void iotcon_delete_query_params(iotcon_query_parameters query_params)
{
	RET_IF(NULL == query_params);

	g_hash_table_destroy(query_params);
}

API char* iotcon_find_query_param(iotcon_query_parameters query_params, const char *key)
{
	return g_hash_table_lookup(query_params, key);
}

API iotcon_observers iotcon_new_observation()
{
	return NULL;
}

API iotcon_observers iotcon_add_observation(iotcon_observers observers,
		iotcon_observation_info_s obs)
{
	iotcon_observation_info_s *obs_node = calloc(1, sizeof(iotcon_observation_info_s));
	RETV_IF(NULL == obs_node, observers);

	obs_node->action = obs.action;
	obs_node->obs_id = obs.obs_id;

	observers = g_list_append(observers, obs_node);

	return observers;
}

API iotcon_observers iotcon_delete_observation(iotcon_observers observers,
		iotcon_observation_info_s obs)
{
	GList *node = NULL;

	node = g_list_find(observers, (gconstpointer)&obs);
	observers = g_list_delete_link(observers, node);

	return observers;
}

API void iotcon_free_observation(iotcon_observers observers)
{
	g_list_free_full(observers, g_free);
}

API char* iotcon_get_resource_uri(iotcon_resource_s resource_s)
{
	return resource_s.resource_uri;
}

API char* iotcon_get_resource_host(iotcon_resource_s resource_s)
{
	return resource_s.resource_host;
}

API iotcon_resource_types iotcon_get_resource_types(iotcon_resource_s resource_s)
{
	return resource_s.resource_types;
}

API iotcon_resource_interfaces iotcon_get_resource_interfaces(
		iotcon_resource_s resource_s)
{
	return resource_s.resource_interfaces;
}

API void iotcon_set_header_options(iotcon_resource_s *resource_s,
		iotcon_header_options options)
{
	iotcon_header_options header_options;
	GHashTableIter iter;
	gpointer key = NULL;
	gpointer value = NULL;

	RET_IF(NULL == resource_s);
	RET_IF(NULL == options);

	header_options = iotcon_new_header_options();
	g_hash_table_iter_init(&iter, options);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		iotcon_add_header_option(header_options, GPOINTER_TO_UINT(key), value);
	}
	resource_s->header_options = header_options;
}

API void iotcon_unset_header_options(iotcon_resource_s *resource_s)
{
	RET_IF(NULL == resource_s);
	iotcon_delete_header_options(resource_s->header_options);
	resource_s->header_options = NULL;
}
