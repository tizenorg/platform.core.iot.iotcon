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
#include <errno.h>

#include <json-glib/json-glib.h>

#include "iotcon.h"

#include "ic-common.h"
#include "ic-struct.h"
#include "ic-repr.h"
#include "ic-repr-value.h"
#include "ic-repr-list.h"

API iotcon_list_h iotcon_list_new(iotcon_repr_types_e type)
{
	iotcon_list_h list;
	errno = 0;

	list = calloc(1, sizeof(struct ic_list_s));
	if (NULL == list) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	list->type = type;

	return list;
}

API iotcon_list_h iotcon_list_append_int(iotcon_list_h list, int ival)
{
	int ret = 0;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_INT != list->type, list, "IOTCON_ERR_PARAM(%d)", list->type);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_INT);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_INT) Fail");
		return list;
	}

	ret = ic_value_set_int(value, ival);
	if (IOTCON_ERR_NONE != ret)  {
		ERR("ic_value_set_int(%d) Fail(%d)", ival, ret);
		ic_repr_free_basic_value(value);
		return list;
	}

	list = ic_list_append(list, value);

	return list;
}

API iotcon_list_h iotcon_list_append_bool(iotcon_list_h list, bool bval)
{
	int ret = 0;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_BOOL != list->type, list, "IOTCON_ERR_PARAM(%d)", list->type);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_BOOL);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_BOOL) Fail");
		return list;
	}

	ret = ic_value_set_bool(value, bval);
	if (IOTCON_ERR_NONE != ret)  {
		ERR("ic_value_set_bool(%d) Fail(%d)", bval, ret);
		ic_repr_free_basic_value(value);
		return list;
	}

	list = ic_list_append(list, value);

	return list;
}

API iotcon_list_h iotcon_list_append_double(iotcon_list_h list, double dbval)
{
	int ret = 0;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_DOUBLE != list->type, list, "IOTCON_ERR_PARAM(%d)",
			list->type);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_DOUBLE);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_DOUBLE) Fail");
		return list;
	}

	ret = ic_value_set_double(value, dbval);
	if (IOTCON_ERR_NONE != ret)  {
		ERR("ic_value_set_double(%lf) Fail(%d)", dbval, ret);
		ic_repr_free_basic_value(value);
		return list;
	}

	list = ic_list_append(list, value);

	return list;
}

API iotcon_list_h iotcon_list_append_str(iotcon_list_h list, const char *strval)
{
	int ret = 0;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_STR != list->type, list, "IOTCON_ERR_PARAM(%d)", list->type);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_STR);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_STR) Fail");
		return list;
	}

	ret = ic_value_set_str(value, strval);
	if (IOTCON_ERR_NONE != ret)  {
		ERR("ic_value_set_str(%s) Fail(%d)", strval, ret);
		ic_repr_free_basic_value(value);
		return list;
	}

	list = ic_list_append(list, value);

	return list;
}

API iotcon_list_h iotcon_list_append_list(iotcon_list_h dest, iotcon_list_h src)
{
	RETV_IF(NULL == dest, NULL);
	RETV_IF(NULL == src, NULL);
	RETVM_IF(IOTCON_TYPE_LIST != dest->type, dest, "IOTCON_ERR_PARAM(%d)", dest->type);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_LIST);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_LIST) Fail");
		return dest;
	}

	ic_val_list_s *real = (ic_val_list_s *)value;
	real->list = src;

	dest = ic_list_append(dest, value);

	return dest;
}

API iotcon_list_h iotcon_list_append_repr(iotcon_list_h list, iotcon_repr_h repr)
{
	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == repr, NULL);
	RETVM_IF(IOTCON_TYPE_REPR != list->type, list, "IOTCON_ERR_PARAM(%d)", list->type);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_REPR);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_REPR) Fail");
		return list;
	}

	ic_val_repr_s *real = (ic_val_repr_s *)value;
	real->repr = repr;

	list = ic_list_append(list, value);

	return list;
}

API iotcon_repr_h iotcon_list_get_nth_repr(iotcon_list_h list, int index)
{
	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	iotcon_value_h value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return NULL;
	}

	ic_val_repr_s *real = (ic_val_repr_s *) value;
	RETVM_IF(IOTCON_TYPE_REPR != real->type, NULL, "IOTCON_ERR_PARAM(%d)", real->type);

	return real->repr;
}

API int iotcon_list_get_length(iotcon_list_h list)
{
	RETV_IF(NULL == list, IOTCON_ERR_PARAM);
	RETV_IF(NULL == list->list, IOTCON_ERR_PARAM);

	return g_list_length(list->list);
}

API iotcon_list_h iotcon_list_get_nth_list(iotcon_list_h list, int index)
{
	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	iotcon_value_h value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return NULL;
	}

	ic_val_list_s *real = (ic_val_list_s *) value;
	RETVM_IF(IOTCON_TYPE_LIST != real->type, NULL, "IOTCON_ERR_PARAM(%d)", real->type);

	return real->list;
}

API iotcon_value_h iotcon_list_get_nth_value(iotcon_list_h list, int index)
{
	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	return g_list_nth_data(list->list, index);
}

int ic_list_remove(iotcon_list_h list, iotcon_value_h val)
{
	RETV_IF(NULL == list, IOTCON_ERR_PARAM);

	list->list = g_list_remove(list->list, val);

	return IOTCON_ERR_NONE;
}

iotcon_list_h ic_list_append(iotcon_list_h list, iotcon_value_h value)
{
	RETV_IF(NULL == list, NULL);

	list->list = g_list_append(list->list, value);
	if (NULL == list->list) {
		ERR("g_list_append() Fail");
		return list;
	}

	return list;
}

API void iotcon_list_foreach(iotcon_list_h list, iotcon_list_fn fn, void *user_data)
{
	RETM_IF(NULL == list, "IOTCON_ERR_PARAM(NULL == list)");
	RETM_IF(NULL == fn, "IOTCON_ERR_PARAM(NULL == fn)");

	g_list_foreach(list->list, (GFunc)fn, user_data);
}

JsonArray* ic_repr_generate_json_array(iotcon_list_h list)
{
	int i;
	int count = 0;
	JsonArray *parray = NULL;
	JsonNode *node_child = NULL;
	int error_code = IOTCON_ERR_NONE;

	JsonObject *obj_child = NULL;
	JsonArray *array_child = NULL;
	iotcon_repr_h repr_child = NULL;
	iotcon_list_h iotlist_child = NULL;
	iotcon_value_h iotvalue_child = NULL;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	count = g_list_length(list->list);
	DBG("list count(%d)", count);

	parray = json_array_new();
	for (i = 0; i < count; i++) {
		iotvalue_child = iotcon_list_get_nth_value(list, i);

		int type = iotcon_value_get_type(iotvalue_child);
		switch (type) {
		case IOTCON_TYPE_INT:
		case IOTCON_TYPE_BOOL:
		case IOTCON_TYPE_DOUBLE:
		case IOTCON_TYPE_STR:
		case IOTCON_TYPE_NULL:
			node_child = ic_repr_generate_json_value(iotvalue_child);
			if (NULL == node_child) {
				ERR("ic_repr_generate_json_value(iotvalue_child) Fail");
				json_array_unref(parray);
				return NULL;
			}
			json_array_add_element(parray, node_child);
			break;
		case IOTCON_TYPE_LIST:
			iotlist_child = iotcon_value_get_list(iotvalue_child);
			array_child = ic_repr_generate_json_array(iotlist_child);
			if (NULL == array_child) {
				ERR("ic_repr_generate_json_array(list_child) Fail");
				json_array_unref(parray);
				return NULL;
			}
			node_child = json_node_new(JSON_NODE_ARRAY);
			json_node_set_array(node_child, array_child);
			json_array_add_element(parray, node_child);
			break;
		case IOTCON_TYPE_REPR:
			repr_child = iotcon_value_get_repr(iotvalue_child);
			obj_child = ic_repr_generate_json_repr(repr_child, &error_code);
			if (NULL == obj_child) {
				ERR("ic_repr_generate_json_repr(repr_child) Fail");
				json_array_unref(parray);
				return NULL;
			}
			node_child = json_node_new(JSON_NODE_OBJECT);
			json_node_set_object(node_child, obj_child);
			json_array_add_element(parray, node_child);
		}
	}
	return parray;
}

iotcon_list_h ic_repr_parse_json_array(JsonArray *parray)
{
	int i;
	int count = json_array_get_length(parray);

	iotcon_list_h list = iotcon_list_new(IOTCON_TYPE_NONE);
//	DBG("array count(%d)", count);

	for (i = 0; i < count; i++) {
		JsonNode *child_node = json_array_get_element(parray, i);
		if (JSON_NODE_HOLDS_NULL(child_node) || JSON_NODE_HOLDS_VALUE(child_node)) {
			iotcon_value_h value = ic_repr_parse_json_value(child_node);
			if (NULL == value) {
				ERR("ic_repr_parse_json_value() Fail(NULL == value)");
				return NULL;
			}

			ic_basic_s *real = (ic_basic_s*)value;
			if (IOTCON_TYPE_NONE != list->type && list->type != real->type) {
				ERR("Type matching Fail(list:%d,value:%d)", list->type, real->type);
				ic_repr_free_basic_value(value);
				iotcon_repr_free_list(list);
				return NULL;
			}

			list = ic_list_append(list, value);
			list->type = real->type;
			continue;
		}
		else if (JSON_NODE_HOLDS_ARRAY(child_node)) {
			JsonArray *child_array = json_node_get_array(child_node);
			iotcon_list_h parsed_list = ic_repr_parse_json_array(child_array);
			if (NULL == parsed_list) {
				ERR("ic_repr_parse_json_array() Fail(NULL == parsed_list)");
				return NULL;
			}

			if (IOTCON_TYPE_NONE != list->type
					&& IOTCON_TYPE_LIST != list->type) {
				ERR("Type matching Fail(%d)", list->type);
				iotcon_repr_free_list(parsed_list);
				iotcon_repr_free_list(list);
				return NULL;
			}

			iotcon_value_h value = ic_value_new(IOTCON_TYPE_LIST);
			ic_value_set_list(value, parsed_list);
			list = ic_list_append(list, value);
			list->type = IOTCON_TYPE_LIST;

			continue;
		}
		else if (JSON_NODE_HOLDS_OBJECT(child_node)) {
			int error_code = IOTCON_ERR_NONE;
			JsonObject *child_obj = json_node_get_object(child_node);
			JsonObject *repr_obj = json_object_get_object_member(child_obj,
					IOTCON_KEY_REP);
			iotcon_repr_h ret_repr = ic_repr_parse_json_obj(repr_obj, &error_code);
			if (NULL == ret_repr) {
				ERR("ic_repr_parse_json_obj() Fail(NULL == ret_repr)");
				return NULL;
			}

			if (IOTCON_TYPE_NONE != list->type
					&& IOTCON_TYPE_REPR != list->type) {
				ERR("Type matching Fail(%d)", list->type);
				iotcon_repr_free(ret_repr);
				iotcon_repr_free_list(list);
				return NULL;
			}

			iotcon_value_h value = ic_value_new(IOTCON_TYPE_REPR);
			ic_value_set_repr(value, ret_repr);
			list = ic_list_append(list, value);
			list->type = IOTCON_TYPE_REPR;
			continue;
		}
	}

	return list;
}

API void iotcon_repr_free_list(iotcon_list_h list)
{
	FN_CALL;
	int count = 0;
	GList *cur = NULL;

	RET_IF(NULL == list);

	if (list->list)
		count = g_list_length(list->list);

	DBG("list count(%d)", count);
	cur = list->list;
	while (cur) {
		ic_repr_free_value(cur->data);
		cur = cur->next;
	}
	free(list);
}
