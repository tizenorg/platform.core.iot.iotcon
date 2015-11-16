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
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <glib.h>

#include "iotcon-types.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource.h"
#include "icl-resource-types.h"
#include "icl-response.h"
#include "icl-list.h"
#include "icl-value.h"
#include "icl-state.h"
#include "icl-representation.h"

void icl_representation_inc_ref_count(iotcon_representation_h val)
{
	RET_IF(NULL == val);
	RETM_IF(val->ref_count < 0, "Invalid Count(%d)", val->ref_count);

	val->ref_count++;
}


static bool _icl_representation_dec_ref_count(iotcon_representation_h val)
{
	bool ret;

	RETV_IF(NULL == val, -1);
	RETVM_IF(val->ref_count <= 0, false, "Invalid Count(%d)", val->ref_count);

	val->ref_count--;
	if (0 == val->ref_count)
		ret = true;
	else
		ret = false;

	return ret;
}


API int iotcon_representation_create(iotcon_representation_h *ret_repr)
{
	errno = 0;
	iotcon_representation_h repr;

	RETV_IF(NULL == ret_repr, IOTCON_ERROR_INVALID_PARAMETER);

	repr = calloc(1, sizeof(struct icl_representation_s));
	if (NULL == repr) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	repr->visibility = (ICL_VISIBILITY_REPR | ICL_VISIBILITY_PROP);
	icl_representation_inc_ref_count(repr);

	*ret_repr = repr;

	return IOTCON_ERROR_NONE;
}


API void iotcon_representation_destroy(iotcon_representation_h repr)
{
	RET_IF(NULL == repr);

	if (false == _icl_representation_dec_ref_count(repr))
		return;

	free(repr->uri_path);

	/* (GDestroyNotify) : iotcon_representation_h is proper type than gpointer */
	g_list_free_full(repr->children, (GDestroyNotify)iotcon_representation_destroy);

	/* null COULD be allowed */
	if (repr->res_types)
		iotcon_resource_types_destroy(repr->res_types);

	/* null COULD be allowed */
	if (repr->state)
		iotcon_state_destroy(repr->state);

	free(repr);
}


API int iotcon_representation_get_uri_path(iotcon_representation_h repr, char **uri_path)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = repr->uri_path;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_uri_path(iotcon_representation_h repr,
		const char *uri_path)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	free(repr->uri_path);
	repr->uri_path = NULL;

	if (NULL == uri_path)
		return IOTCON_ERROR_INVALID_PARAMETER;

	repr->uri_path = strdup(uri_path);
	if (NULL == repr->uri_path) {
		ERR("strdup() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_resource_types(iotcon_representation_h repr,
		iotcon_resource_types_h *types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = repr->res_types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_resource_types(iotcon_representation_h repr,
		iotcon_resource_types_h types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_resource_types_destroy(repr->res_types);
	repr->res_types = NULL;

	if (types)
		repr->res_types = icl_resource_types_ref(types);

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_resource_interfaces(iotcon_representation_h repr,
		int *ifaces)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = repr->interfaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_resource_interfaces(iotcon_representation_h repr,
		int ifaces)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	RETV_IF(ifaces <= IOTCON_INTERFACE_NONE || IC_INTERFACE_MAX < ifaces,
			IOTCON_ERROR_INVALID_PARAMETER);

	repr->interfaces = ifaces;

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_set_state(iotcon_representation_h repr,
		iotcon_state_h state)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	if (repr->state) {
		ERR("state already set. Remove first !");
		return IOTCON_ERROR_ALREADY;
	}

	icl_state_inc_ref_count(state);
	repr->state = state;

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_get_state(iotcon_representation_h repr,
		iotcon_state_h *state)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	*state = repr->state;

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_del_state(iotcon_representation_h repr)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	if (repr->state)
		iotcon_state_destroy(repr->state);

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_append_child(iotcon_representation_h parent,
		iotcon_representation_h child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	icl_representation_inc_ref_count(child);
	parent->children = g_list_append(parent->children, child);

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_remove_child(iotcon_representation_h parent,
		iotcon_representation_h child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	parent->children = g_list_remove(parent->children, child);

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_foreach_children(iotcon_representation_h parent,
		iotcon_children_cb cb, void *user_data)
{
	GList *list, *next;

	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	list = parent->children;
	while (list) {
		next = list->next;
		if (IOTCON_FUNC_STOP == cb(list->data, user_data))
			break;
		list = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_children_count(iotcon_representation_h parent,
		unsigned int *count)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == parent->children, IOTCON_ERROR_INVALID_PARAMETER);

	*count = g_list_length(parent->children);

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_nth_child(iotcon_representation_h parent, int pos,
		iotcon_representation_h *child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == parent->children, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	*child = g_list_nth_data(parent->children, pos);
	if (NULL == *child) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_foreach(iotcon_state_h state, iotcon_state_cb cb, void *user_data)
{
	GHashTableIter iter;
	gpointer key;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_iter_init(&iter, state->hash_table);
	while (g_hash_table_iter_next(&iter, &key, NULL)) {
		if (IOTCON_FUNC_STOP == cb(state, key, user_data))
			break;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_state_get_keys_count(iotcon_state_h state, unsigned int *count)
{
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state->hash_table, IOTCON_ERROR_INVALID_PARAMETER);

	*count = g_hash_table_size(state->hash_table);

	return IOTCON_ERROR_NONE;
}


void icl_state_clone_foreach(char *key, iotcon_value_h src_val, iotcon_state_h dest_state)
{
	FN_CALL;
	int type, ret;
	iotcon_value_h value, copied_val;
	iotcon_list_h child_list, copied_list;
	iotcon_state_h child_state;
	iotcon_state_h copied_state = NULL;

	type = src_val->type;
	switch (type) {
	case IOTCON_TYPE_INT:
	case IOTCON_TYPE_BOOL:
	case IOTCON_TYPE_DOUBLE:
	case IOTCON_TYPE_STR:
	case IOTCON_TYPE_NULL:
		copied_val = icl_value_clone(src_val);
		if (NULL == copied_val) {
			ERR("icl_value_clone() Fail");
			return;
		}

		icl_state_set_value(dest_state, key, copied_val);
		break;
	case IOTCON_TYPE_LIST:
		ret = icl_value_get_list(src_val, &child_list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_value_get_list() Fail(%d)", ret);
			return;
		}

		copied_list = icl_list_clone(child_list);
		if (NULL == copied_list) {
			ERR("icl_list_clone() Fail");
			return;
		}

		value = icl_value_create_list(copied_list);
		if (NULL == value) {
			ERR("icl_value_create_list() Fail");
			iotcon_list_destroy(copied_list);
			return;
		}

		icl_state_set_value(dest_state, key, value);
		break;
	case IOTCON_TYPE_STATE:
		ret = icl_value_get_state(src_val, &child_state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_value_get_state() Fail(%d)", ret);
			return;
		}

		g_hash_table_foreach(child_state->hash_table, (GHFunc)icl_state_clone_foreach,
				copied_state);

		value = icl_value_create_state(copied_state);
		if (NULL == value) {
			ERR("icl_value_create_state(%p) Fail", copied_state);
			return;
		}

		icl_state_set_value(dest_state, key, value);
		break;
	default:
		ERR("Invalid type(%d)", type);
		return;
	}
}

API int iotcon_representation_clone(const iotcon_representation_h src,
		iotcon_representation_h *dest)
{
	FN_CALL;
	int ret;
	GList *node;
	iotcon_resource_types_h list;
	iotcon_state_h ori_state;
	iotcon_state_h cloned_state = NULL;
	iotcon_representation_h cloned_repr, copied_repr;

	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_representation_create(&cloned_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return ret;
	}

	if (src->uri_path) {
		cloned_repr->uri_path = strdup(src->uri_path);
		if (NULL == cloned_repr->uri_path) {
			ERR("strdup() Fail");
			iotcon_representation_destroy(cloned_repr);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
	}

	if (src->interfaces)
		cloned_repr->interfaces = src->interfaces;

	if (src->res_types) {
		ret = iotcon_resource_types_clone(src->res_types, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_types_clone() Fail");
			iotcon_representation_destroy(cloned_repr);
			return ret;
		}
		cloned_repr->res_types = list;
	}

	for (node = g_list_first(src->children); node; node = node->next) {
		ret = iotcon_representation_clone((iotcon_representation_h)node->data,
				&copied_repr);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_clone(child) Fail(%d)", ret);
			iotcon_representation_destroy(cloned_repr);
			return ret;
		}
		cloned_repr->children = g_list_append(cloned_repr->children, copied_repr);
	}

	ori_state = src->state;
	if (ori_state->hash_table) {
		ret = iotcon_state_create(&cloned_state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_create() Fail");
			iotcon_representation_destroy(cloned_repr);
			return ret;
		}
		g_hash_table_foreach(ori_state->hash_table, (GHFunc)icl_state_clone_foreach,
				cloned_state);
		ret = iotcon_representation_set_state(cloned_repr, cloned_state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_set_state() Fail");
			iotcon_state_destroy(cloned_state);
			iotcon_representation_destroy(cloned_repr);
			return ret;
		}
		iotcon_state_destroy(cloned_state);
	}

	*dest = cloned_repr;

	return IOTCON_ERROR_NONE;
}
