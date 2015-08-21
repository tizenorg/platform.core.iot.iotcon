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
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <glib.h>

#include "iotcon-struct.h"
#include "iotcon-representation.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource.h"
#include "icl-resource-types.h"
#include "icl-response.h"
#include "icl-repr-list.h"
#include "icl-repr-value.h"
#include "icl-repr-obj.h"
#include "icl-repr.h"

void icl_repr_inc_ref_count(iotcon_repr_h val)
{
	RET_IF(NULL == val);
	RETM_IF(val->ref_count < 0, "Invalid Count(%d)", val->ref_count);

	val->ref_count++;
}

static bool _icl_repr_dec_ref_count(iotcon_repr_h val)
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

API iotcon_repr_h iotcon_repr_new()
{
	iotcon_repr_h ret_val;
	errno = 0;

	ret_val = calloc(1, sizeof(struct icl_repr_s));
	if (NULL == ret_val) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	ret_val->hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, free,
			icl_value_free);
	icl_repr_inc_ref_count(ret_val);
	ret_val->visibility = (ICL_VISIBILITY_REPR | ICL_VISIBILITY_PROP);

	return ret_val;
}

API int iotcon_repr_get_uri_path(iotcon_repr_h repr, const char **uri_path)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = repr->uri_path;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_set_uri_path(iotcon_repr_h repr, const char *uri_path)
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

API int iotcon_repr_get_resource_types(iotcon_repr_h repr, iotcon_resource_types_h *types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = repr->res_types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_set_resource_types(iotcon_repr_h repr, iotcon_resource_types_h types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_resource_types_free(repr->res_types);
	repr->res_types = NULL;

	if (types)
		repr->res_types = icl_resource_types_ref(types);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_get_resource_interfaces(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, IOTCON_INTERFACE_NONE);

	return repr->interfaces;
}

API int iotcon_repr_set_resource_interfaces(iotcon_repr_h repr, int ifaces)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	RETV_IF(ifaces <= IOTCON_INTERFACE_NONE || IC_INTERFACE_MAX < ifaces,
			IOTCON_ERROR_INVALID_PARAMETER);

	repr->interfaces = ifaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_append_child(iotcon_repr_h parent, iotcon_repr_h child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	icl_repr_inc_ref_count(child);
	parent->children = g_list_append(parent->children, child);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_foreach_children(iotcon_repr_h parent, iotcon_children_fn fn,
		void *user_data)
{
	GList *list, *next;

	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	list = parent->children;
	while (list) {
		next = list->next;
		if (IOTCON_FUNC_STOP == fn(list->data, user_data))
			break;
		list = next;
	}

	return IOTCON_ERROR_NONE;
}

API unsigned int iotcon_repr_get_children_count(iotcon_repr_h parent)
{
	RETV_IF(NULL == parent, 0);
	RETV_IF(NULL == parent->children, 0);

	return g_list_length(parent->children);
}

API int iotcon_repr_get_nth_child(iotcon_repr_h parent, int pos, iotcon_repr_h *child)
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

API int iotcon_repr_foreach(iotcon_repr_h repr, iotcon_repr_fn fn, void *user_data)
{
	GHashTableIter iter;
	gpointer key;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_iter_init(&iter, repr->hash_table);
	while (g_hash_table_iter_next(&iter, &key, NULL)) {
		if (IOTCON_FUNC_STOP == fn(repr, key, user_data))
			break;
	}

	return IOTCON_ERROR_NONE;
}

API unsigned int iotcon_repr_get_keys_count(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, 0);
	RETV_IF(NULL == repr->hash_table, 0);

	return g_hash_table_size(repr->hash_table);
}


API void iotcon_repr_free(iotcon_repr_h repr)
{
	RET_IF(NULL == repr);

	if (false == _icl_repr_dec_ref_count(repr))
		return;

	free(repr->uri_path);

	/* (GDestroyNotify) : iotcon_repr_h is proper type than gpointer */
	g_list_free_full(repr->children, (GDestroyNotify)iotcon_repr_free);

	/* null COULD be allowed */
	if (repr->res_types)
		iotcon_resource_types_free(repr->res_types);
	g_hash_table_destroy(repr->hash_table);
	free(repr);
}

static void _icl_repr_obj_clone(char *key, iotcon_value_h src_val, iotcon_repr_h dest_repr)
{
	FN_CALL;
	int type, ret;
	iotcon_value_h value, copied_val;
	iotcon_list_h child_list, copied_list;
	iotcon_repr_h child_repr, copied_repr;

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

		icl_obj_set_value(dest_repr, key, copied_val);
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

		value = icl_value_new_list(copied_list);
		if (NULL == value) {
			ERR("icl_value_new_list() Fail");
			iotcon_list_free(copied_list);
			return;
		}

		icl_obj_set_value(dest_repr, key, value);
		break;
	case IOTCON_TYPE_REPR:
		ret = icl_value_get_repr(src_val, &child_repr);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_value_get_repr() Fail(%d)", ret);
			return;
		}

		copied_repr = iotcon_repr_clone(child_repr);
		if (NULL == copied_repr) {
			ERR("icl_list_clone() Fail");
			return;
		}

		value = icl_value_new_repr(copied_repr);
		if (NULL == value) {
			ERR("icl_value_new_repr(%p) Fail", copied_repr);
			return;
		}

		icl_obj_set_value(dest_repr, key, value);
		break;
	default:
		ERR("Invalid type(%d)", type);
		return;
	}
}

API iotcon_repr_h iotcon_repr_clone(const iotcon_repr_h src)
{
	FN_CALL;
	GList *node;
	iotcon_repr_h dest, copied_repr;
	iotcon_resource_types_h list;

	RETV_IF(NULL == src, NULL);

	dest = iotcon_repr_new();
	if (NULL == dest) {
		ERR("iotcon_repr_new() Fail");
		return NULL;
	}

	if (src->uri_path) {
		dest->uri_path = strdup(src->uri_path);
		if (NULL == dest->uri_path) {
			ERR("strdup() Fail");
			iotcon_repr_free(dest);
			return NULL;
		}
	}

	if (src->interfaces)
		dest->interfaces = src->interfaces;

	if (src->res_types) {
		list = iotcon_resource_types_clone(src->res_types);
		if (NULL == list) {
			ERR("iotcon_resource_types_clone() Fail");
			iotcon_repr_free(dest);
			return NULL;
		}
		dest->res_types = list;
	}

	for (node = g_list_first(src->children); node; node = node->next) {
		copied_repr = iotcon_repr_clone((iotcon_repr_h)node->data);
		if (NULL == copied_repr) {
			ERR("iotcon_repr_clone(child) Fail");
			iotcon_repr_free(dest);
			return NULL;
		}
		dest->children = g_list_append(dest->children, copied_repr);
	}

	g_hash_table_foreach(src->hash_table, (GHFunc)_icl_repr_obj_clone, dest);

	return dest;
}
