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
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "iotcon-internal.h"
#include "ic-dbus.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-representation.h"
#include "icl-list.h"
#include "icl-value.h"
#include "icl-remote-resource.h"

static int _caching_compare_state(iotcon_state_h state1, iotcon_state_h state2);
static int _caching_compare_value(iotcon_value_h val1, iotcon_value_h val2);
static int _caching_compare_repr(iotcon_representation_h repr1,
		iotcon_representation_h repr2);

static int _caching_compare_glist(GList *list1, GList *list2, GCompareFunc func)
{
	GList *c;

	if (NULL == list1 || NULL == list2)
		return !!(list1 - list2);

	if (g_list_length(list1) != g_list_length(list2))
		return 1;

	for (c = list1; c; c = c->next) {
		if (NULL == g_list_find_custom(list2, c->data, func))
			return 1;
	}

	return IC_EQUAL;
}

static gint _caching_find_value_custom(gconstpointer a, gconstpointer b)
{
	return _caching_compare_value((iotcon_value_h)a, (iotcon_value_h)b);
}

static int _caching_compare_list(iotcon_list_h list1, iotcon_list_h list2)
{
	if (NULL == list1 || NULL == list2)
		return !!(list1 - list2);

	if (list1->type != list2->type)
		return 1;

	return _caching_compare_glist(list1->list, list2->list, _caching_find_value_custom);
}

static int _caching_compare_value(iotcon_value_h val1, iotcon_value_h val2)
{
	if (NULL == val1 || NULL == val2)
		return !!(val1 - val2);

	if (val1->type != val2->type)
		return 1;

	switch (val1->type) {
	case IOTCON_TYPE_INT:
		return (((icl_basic_s *)val1)->val.i == ((icl_basic_s *)val2)->val.i)? 0: 1;
	case IOTCON_TYPE_BOOL:
		return (((icl_basic_s *)val1)->val.b == ((icl_basic_s *)val2)->val.b)? 0: 1;
	case IOTCON_TYPE_DOUBLE:
		return (((icl_basic_s *)val1)->val.d == ((icl_basic_s *)val2)->val.d)? 0: 1;
	case IOTCON_TYPE_STR:
		return g_strcmp0(((icl_basic_s *)val1)->val.s, ((icl_basic_s *)val2)->val.s);
	case IOTCON_TYPE_NULL:
		return IC_EQUAL;
	case IOTCON_TYPE_LIST:
		return _caching_compare_list(((icl_val_list_s *)val1)->list,
				((icl_val_list_s *)val2)->list);
	case IOTCON_TYPE_STATE:
		return _caching_compare_state(((icl_val_state_s *)val1)->state,
				((icl_val_state_s *)val2)->state);
	case IOTCON_TYPE_NONE:
	default:
		ERR("Invalid type (%d)", val1->type);
	}
	return IC_EQUAL;
}

static int _caching_compare_state(iotcon_state_h state1, iotcon_state_h state2)
{
	int ret;
	GHashTableIter iter;
	gpointer key, value;

	if (NULL == state1 || NULL == state2)
		return !!(state1 - state2);

	/* compare state */
	if (g_hash_table_size(state1->hash_table) != g_hash_table_size(state2->hash_table))
		return 1;

	g_hash_table_iter_init(&iter, state1->hash_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		iotcon_value_h val1 = value;
		iotcon_value_h val2 = g_hash_table_lookup(state2->hash_table, key);
		ret = _caching_compare_value(val1, val2);
		if (IC_EQUAL != ret)
			return 1;
	}
	return IC_EQUAL;
}

static gint _caching_find_repr_custom(gconstpointer a, gconstpointer b)
{
	iotcon_representation_h repr1 = (iotcon_representation_h)a;
	iotcon_representation_h repr2 = (iotcon_representation_h)b;
	return _caching_compare_repr(repr1, repr2);
}

static gint _caching_find_resource_types_custom(gconstpointer a, gconstpointer b)
{
	return g_strcmp0(a, b);
}

static int _caching_compare_resource_types(iotcon_resource_types_h types1,
		iotcon_resource_types_h types2)
{
	int ret;

	if (NULL == types1 || NULL == types2)
		return !!(types1 - types2);

	ret = _caching_compare_glist(types1->type_list, types2->type_list,
			_caching_find_resource_types_custom);
	if (IC_EQUAL != ret)
		return 1;

	return IC_EQUAL;
}

/* return 0 on same */
static int _caching_compare_repr(iotcon_representation_h repr1,
		iotcon_representation_h repr2)
{
	int ret;

	if (NULL == repr1 || NULL == repr2)
		return !!(repr1 - repr2);

	/* compare interface */
	if (repr1->interfaces != repr2->interfaces)
		return 1;

	/* compare visibility */
	if (repr1->visibility != repr2->visibility)
		return 1;

	/* compare uri_path */
	if (IC_STR_EQUAL != g_strcmp0(repr1->uri_path, repr2->uri_path))
		return 1;

	ret = _caching_compare_resource_types(repr1->res_types, repr2->res_types);
	if (IC_EQUAL != ret)
		return 1;

	/* compare state */
	ret = _caching_compare_state(repr1->state, repr2->state);
	if (IC_EQUAL != ret)
		return 1;

	/* compare children */
	ret = _caching_compare_glist(repr1->children, repr2->children,
			_caching_find_repr_custom);
	if (IC_EQUAL != ret)
		return 1;

	return IC_EQUAL;
}

static void _caching_get_cb(iotcon_remote_resource_h resource,
		iotcon_error_e err,
		iotcon_request_type_e request_type,
		iotcon_response_h resp,
		void *user_data)
{
	int ret;
	iotcon_response_result_e result;
	iotcon_representation_h repr = NULL;
	iotcon_representation_h cloned_repr;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->caching_handle);
	RETM_IF(IOTCON_ERROR_NONE != err, "_caching_get() Fail(%d)", err);

	ret = iotcon_response_get_result(resp, &result);
	if (IOTCON_ERROR_NONE != ret || IOTCON_RESPONSE_RESULT_OK != result) {
		ERR("Invalid result (%d)", result);
		return;
	}

	ret = iotcon_response_get_representation(resp, &repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_response_get_representation() Fail(%d)", ret);
		return;
	}

	ret = _caching_compare_repr(resource->caching_handle->repr, repr);
	if (IC_EQUAL == ret) { /* same */
		DBG("Not changed");
		return;
	}

	ret = iotcon_representation_clone(repr, &cloned_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_clone() Fail(%d)", ret);
		return;
	}

	if (resource->caching_handle->repr)
		iotcon_representation_destroy(resource->caching_handle->repr);

	resource->caching_handle->repr = cloned_repr;

	if (resource->caching_handle->cb)
		resource->caching_handle->cb(resource, repr, resource->caching_handle->user_data);
}


static void _caching_observe_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	int ret;
	iotcon_remote_resource_h resource = user_data;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->caching_handle);

	ret = iotcon_remote_resource_get(resource, NULL, _caching_get_cb, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_remote_resource_get() for caching Fail(%d)", ret);
}


static void _caching_observe_cleanup(iotcon_remote_resource_h resource)
{
	resource->caching_handle->observe_handle = 0;
	resource->caching_handle->observe_sub_id = 0;
}


static int _caching_observer_start(iotcon_remote_resource_h resource)
{
	int ret;
	int64_t handle = 0;
	unsigned int sub_id = 0;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_remote_resource_observer_start(resource, IOTCON_OBSERVE_IGNORE_OUT_OF_ORDER, NULL,
			_caching_observe_cb, resource, _caching_observe_cleanup, &sub_id, &handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_remote_resource_observer_start() Fail(%d)", ret);
		return ret;
	}

	resource->caching_handle->observe_sub_id = sub_id;
	resource->caching_handle->observe_handle = handle;

	return IOTCON_ERROR_NONE;
}


static int _caching_observer_stop(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (0 == resource->caching_handle->observe_handle) {
		ERR("It doesn't have a caching observe handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = icl_remote_resource_stop_observing(resource,
			NULL,
			resource->caching_handle->observe_handle,
			resource->caching_handle->observe_sub_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_remote_resource_stop_observing() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


static gboolean _caching_get_timer(gpointer user_data)
{
	int ret;
	iotcon_remote_resource_h resource = user_data;

	RETV_IF(NULL == resource, G_SOURCE_REMOVE);

	ret = iotcon_remote_resource_get(resource, NULL, _caching_get_cb, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_remote_resource_get() for caching Fail(%d)", ret);

	return G_SOURCE_CONTINUE;
}


API int iotcon_remote_resource_start_caching(iotcon_remote_resource_h resource,
		iotcon_remote_resource_cached_representation_changed_cb cb, void *user_data)
{
	int ret, time_interval;
	unsigned int get_timer_id;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (resource->caching_handle) {
		ERR("Already Start Caching");
		return IOTCON_ERROR_ALREADY;
	}

	INFO("Start Caching");

	resource->caching_handle = calloc(1, sizeof(struct icl_remote_resource_caching));
	if (NULL == resource->caching_handle) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	_caching_get_timer(resource);

	/* OBSERVE METHOD */
	ret = _caching_observer_start(resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_caching_observer_start() Fail(%d)", ret);
		free(resource->caching_handle);
		resource->caching_handle = NULL;
		return ret;
	}

	/* GET METHOD */
	resource->caching_handle->cb = cb;
	resource->caching_handle->user_data = user_data;

	ret = iotcon_remote_resource_get_time_interval(&time_interval);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remote_resource_get_time_interval() Fail(%d)", ret);
		return ret;
	}

	get_timer_id = g_timeout_add_seconds(time_interval, _caching_get_timer, resource);
	resource->caching_handle->get_timer_id = get_timer_id;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_stop_caching(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == resource->caching_handle) {
		ERR("Not Cached");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	INFO("Stop Caching");

	/* GET METHOD */
	g_source_remove(resource->caching_handle->get_timer_id);

	if (resource->caching_handle->repr) {
		iotcon_representation_destroy(resource->caching_handle->repr);
		resource->caching_handle->repr = NULL;
	}

	/* OBSERVE METHOD */
	ret = _caching_observer_stop(resource);
	if (IOTCON_ERROR_NONE == ret) {
		ERR("_caching_observer_stop() Fail(%d)", ret);
		return ret;
	}

	free(resource->caching_handle);
	resource->caching_handle = NULL;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_get_cached_representation(
		iotcon_remote_resource_h resource,
		iotcon_representation_h *representation)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == representation, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == resource->caching_handle->repr) {
		ERR("No Caching Representation");
		return IOTCON_ERROR_NO_DATA;
	}

	*representation = resource->caching_handle->repr;

	return IOTCON_ERROR_NONE;
}

