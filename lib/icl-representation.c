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

static int _icl_repr_compare_state_value(struct icl_value_s *val1,
		struct icl_value_s *val2);
static int _icl_repr_compare_state(iotcon_state_h state1, iotcon_state_h state2);

iotcon_representation_h icl_representation_ref(iotcon_representation_h repr)
{
	RETV_IF(NULL == repr, NULL);
	RETV_IF(repr->ref_count <= 0, NULL);

	repr->ref_count++;

	return repr;
}


API int iotcon_representation_create(iotcon_representation_h *ret_repr)
{
	errno = 0;
	iotcon_representation_h repr;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == ret_repr, IOTCON_ERROR_INVALID_PARAMETER);

	repr = calloc(1, sizeof(struct icl_representation_s));
	if (NULL == repr) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	repr->visibility = (ICL_VISIBILITY_REPR | ICL_VISIBILITY_PROP);
	repr->ref_count = 1;

	*ret_repr = repr;

	return IOTCON_ERROR_NONE;
}


API void iotcon_representation_destroy(iotcon_representation_h repr)
{
	RET_IF(NULL == repr);

	repr->ref_count--;

	if (0 != repr->ref_count)
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
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = repr->uri_path;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_uri_path(iotcon_representation_h repr,
		const char *uri_path)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	free(repr->uri_path);
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
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = repr->res_types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_resource_types(iotcon_representation_h repr,
		iotcon_resource_types_h types)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	if (types)
		types = icl_resource_types_ref(types);

	if (repr->res_types)
		iotcon_resource_types_destroy(repr->res_types);

	repr->res_types = types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_resource_interfaces(iotcon_representation_h repr,
		int *ifaces)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = repr->interfaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_resource_interfaces(iotcon_representation_h repr,
		int ifaces)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	RETVM_IF(ifaces <= IOTCON_INTERFACE_NONE || IC_INTERFACE_MAX < ifaces,
			IOTCON_ERROR_INVALID_PARAMETER, "ifaces(%d)", ifaces);

	repr->interfaces = ifaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_state(iotcon_representation_h repr,
		iotcon_state_h state)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	if (state)
		state = icl_state_ref(state);

	if (repr->state)
		iotcon_state_destroy(repr->state);

	repr->state = state;

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_get_state(iotcon_representation_h repr,
		iotcon_state_h *state)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	*state = repr->state;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_add_child(iotcon_representation_h parent,
		iotcon_representation_h child)
{
	iotcon_representation_h repr;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	repr = icl_representation_ref(child);

	parent->children = g_list_append(parent->children, repr);

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_remove_child(iotcon_representation_h parent,
		iotcon_representation_h child)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	parent->children = g_list_remove(parent->children, child);

	iotcon_representation_destroy(child);

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_foreach_children(iotcon_representation_h parent,
		iotcon_children_cb cb, void *user_data)
{
	GList *list, *next;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
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
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == count, IOTCON_ERROR_INVALID_PARAMETER);
	if (NULL == parent->children)
		*count = 0;
	else
		*count = g_list_length(parent->children);

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_nth_child(iotcon_representation_h parent, int pos,
		iotcon_representation_h *child)
{
	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
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


API int iotcon_representation_clone(const iotcon_representation_h src,
		iotcon_representation_h *dest)
{
	FN_CALL;
	int ret;
	GList *node;
	iotcon_resource_types_h list;
	iotcon_representation_h cloned_repr, copied_repr;

	RETV_IF(false == ic_utils_check_oic_feature_supported(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

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

	if (src->children) {
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
	}

	if (src->state) {
		ret = iotcon_state_clone(src->state, &cloned_repr->state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_clone() Fail(%d)", ret);
			iotcon_representation_destroy(cloned_repr);
			return ret;
		}
	}

	*dest = cloned_repr;

	return IOTCON_ERROR_NONE;
}

static int _icl_repr_compare_string(const char *str1, const char *str2)
{
	if (NULL == str1 || NULL == str2)
		return !!(str1 - str2);

	return strcmp(str1, str2);
}

static int _icl_repr_compare_str(gconstpointer p1, gconstpointer p2)
{
	return _icl_repr_compare_string(p1, p2);
}


static int _icl_repr_compare_string_list(GList *list1, GList *list2)
{
	GList *c;

	if (NULL == list1 || NULL == list2)
		return !!(list1 - list2);

	if (g_list_length(list1) != g_list_length(list2))
		return 1;

	for (c = list1; c; c = c->next) {
		if (NULL == g_list_find_custom(list2, c->data, _icl_repr_compare_str)) /* not found */
			return 1;
	}

	return IC_EQUAL;
}

static int _icl_repr_compare_resource_types(iotcon_resource_types_h types1,
		iotcon_resource_types_h types2)
{
	int ret;

	if (NULL == types1 || NULL == types2)
		return !!(types1 - types2);

	ret = _icl_repr_compare_string_list(types1->type_list, types2->type_list);
	if (IC_EQUAL != ret)
		return ret;

	return IC_EQUAL;
}

static int _icl_repr_compare_state_value_custom(gconstpointer p1, gconstpointer p2)
{
	struct icl_value_s *val1 = (struct icl_value_s *)p1;
	struct icl_value_s *val2 = (struct icl_value_s *)p2;
	return _icl_repr_compare_state_value(val1, val2);
}

static int _icl_repr_compare_state_list_value(struct icl_list_s *list1,
		struct icl_list_s *list2)
{
	GList *c;

	if (NULL == list1 || NULL == list2)
		return !!(list1 - list2);

	if (list1->type != list2->type)
		return 1;

	if (NULL == list1->list || NULL == list2->list)
		return !!(list1->list - list2->list);

	for (c = list1->list; c; c = c->next) {
		g_list_find_custom(list2->list, c->data, _icl_repr_compare_state_value_custom);
	}

	return IC_EQUAL;
}

static int _icl_repr_compare_state_value(struct icl_value_s *val1,
		struct icl_value_s *val2)
{
	int i;
	icl_val_byte_str_s *val_bstr1, *val_bstr2;

	if (NULL == val1 || NULL == val2)
		return !!(val1 - val2);

	if (val1->type != val2->type)
		return 1;

	switch (val1->type) {
	case IOTCON_TYPE_BOOL:
		if (((icl_basic_s *)val1)->val.b != ((icl_basic_s *)val2)->val.b)
			return 1;
		break;
	case IOTCON_TYPE_INT:
		if (((icl_basic_s *)val1)->val.i != ((icl_basic_s *)val2)->val.i)
			return 1;
		break;
	case IOTCON_TYPE_DOUBLE:
		if (((icl_basic_s *)val1)->val.d != ((icl_basic_s *)val2)->val.d)
			return 1;
		break;
	case IOTCON_TYPE_STR:
		return g_strcmp0(((icl_basic_s *)val1)->val.s, ((icl_basic_s *)val2)->val.s);
	case IOTCON_TYPE_BYTE_STR:
		val_bstr1 = (icl_val_byte_str_s *)val1;
		val_bstr2 = (icl_val_byte_str_s *)val2;

		if (val_bstr1->len != val_bstr2->len)
			return 1;

		for (i = 0; i < val_bstr1->len; i++) {
			if (val_bstr1->s[i] != val_bstr2->s[i])
				return 1;
		}
		break;
	case IOTCON_TYPE_STATE:
		return _icl_repr_compare_state(((icl_val_state_s *)val1)->state,
				((icl_val_state_s *)val2)->state);
	case IOTCON_TYPE_LIST:
		return _icl_repr_compare_state_list_value(((icl_val_list_s*)val1)->list,
				((icl_val_list_s*)val2)->list);
	case IOTCON_TYPE_NULL:
		return IC_EQUAL;
	default:
		ERR("Invalid type(%d)", val1->type);
		return 1;
	}

	return IC_EQUAL;
}

static int _icl_repr_compare_state(iotcon_state_h state1, iotcon_state_h state2)
{
	int ret;
	gpointer key, value1, value2;
	GHashTableIter iter;
	struct icl_value_s *state_val1, *state_val2;

	if (NULL == state1 || NULL == state2)
		return !!(state1 - state2);

	if (NULL == state1->hash_table || NULL == state2->hash_table)
		return !!(IC_POINTER_TO_INT64(state1->hash_table) -
				IC_POINTER_TO_INT64(state2->hash_table));

	g_hash_table_iter_init(&iter, state1->hash_table);
	while (g_hash_table_iter_next(&iter, &key, &value1)) {
		value2 = g_hash_table_lookup(state2->hash_table, key);
		state_val1 = (struct icl_value_s *)value1;
		state_val2 = (struct icl_value_s *)value2;
		ret = _icl_repr_compare_state_value(state_val1, state_val2);
		if (IC_EQUAL != ret)
			return ret;
	}

	return IC_EQUAL;
}

static int _icl_repr_compare_custom(gconstpointer p1, gconstpointer p2)
{
	iotcon_representation_h repr1 = (iotcon_representation_h)p1;
	iotcon_representation_h repr2 = (iotcon_representation_h)p2;
	return icl_representation_compare(repr1, repr2);
}

static int _icl_repr_compare_children(GList *children1, GList *children2)
{
	GList *c;

	if (NULL == children1 || NULL == children2)
		return !!(children1 - children2);

	if (g_list_length(children1) != g_list_length(children2))
		return 1;

	for (c = children1; c; c = c->next) {
		if (NULL == g_list_find_custom(children2, c->data, _icl_repr_compare_custom)) /* not found */
			return 1;
	}

	return IC_EQUAL;
}

int icl_representation_compare(iotcon_representation_h repr1,
		iotcon_representation_h repr2)
{
	int ret;

	if (NULL == repr1 || NULL == repr2)
		return !!(repr1 - repr2);

	/* interfaces */
	if (repr1->interfaces != repr2->interfaces)
		return 1;

	/* uri path */
	ret = _icl_repr_compare_string(repr1->uri_path, repr2->uri_path);
	if (IC_EQUAL != ret)
		return ret;

	/* resource types */
	ret = _icl_repr_compare_resource_types(repr1->res_types, repr2->res_types);
	if (IC_EQUAL != ret)
		return ret;

	/* state */
	ret = _icl_repr_compare_state(repr1->state, repr2->state);
	if (IC_EQUAL != ret)
		return ret;

	/* children */
	ret = _icl_repr_compare_children(repr1->children, repr2->children);
	if (IC_EQUAL != ret)
		return ret;

	return IC_EQUAL;
}

