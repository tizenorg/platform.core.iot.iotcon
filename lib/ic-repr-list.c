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

#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "iotcon-representation.h"
#include "ic-common.h"
#include "ic-repr-obj.h"
#include "ic-repr.h"
#include "ic-repr-value.h"
#include "ic-repr-list.h"

API iotcon_list_h iotcon_list_new(iotcon_repr_types_e type)
{
	iotcon_list_h list;

	list = calloc(1, sizeof(struct ic_list_s));
	if (NULL == list) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	list->type = type;

	return list;
}


API iotcon_list_h iotcon_list_insert_int(iotcon_list_h list, int val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_INT != list->type, list, "Invalid Type(%d)", list->type);

	value = ic_value_new_int(val);
	if (NULL == value) {
		ERR("ic_value_new_int(%d) Fail", val);
		return list;
	}

	list = ic_list_insert(list, value, pos);

	return list;
}


API iotcon_list_h iotcon_list_insert_bool(iotcon_list_h list, bool val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_BOOL != list->type, list, "Invalid Type(%d)", list->type);

	value = ic_value_new_bool(val);
	if (NULL == value) {
		ERR("ic_value_new_bool(%d) Fail", val);
		return list;
	}

	list = ic_list_insert(list, value, pos);

	return list;
}


API iotcon_list_h iotcon_list_insert_double(iotcon_list_h list, double val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_DOUBLE != list->type, list, "Invalid Type(%d)", list->type);

	value = ic_value_new_double(val);
	if (NULL == value) {
		ERR("ic_value_new_double(%f) Fail", val);
		return list;
	}

	list = ic_list_insert(list, value, pos);

	return list;
}


API iotcon_list_h iotcon_list_insert_str(iotcon_list_h list, char *val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETVM_IF(IOTCON_TYPE_STR != list->type, list, "Invalid Type(%d)", list->type);

	value = ic_value_new_str(val);
	if (NULL == value) {
		ERR("ic_value_new_str(%s) Fail", val);
		return list;
	}

	list = ic_list_insert(list, value, pos);

	return list;
}


API iotcon_list_h iotcon_list_insert_list(iotcon_list_h list, iotcon_list_h val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == val, NULL);
	RETVM_IF(IOTCON_TYPE_LIST != list->type, list, "Invalid Type(%d)", list->type);

	value = ic_value_new_list(val);
	if (NULL == value) {
		ERR("ic_value_new_list(%p) Fail", val);
		return list;
	}

	list = ic_list_insert(list, value, pos);

	return list;
}


API iotcon_list_h iotcon_list_insert_repr(iotcon_list_h list, iotcon_repr_h val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == val, NULL);
	RETVM_IF(IOTCON_TYPE_REPR != list->type, list, "Invalid Type(%d)", list->type);

	value = ic_value_new_repr(val);
	if (NULL == value) {
		ERR("ic_value_new_repr(%p) Fail", val);
		return list;
	}

	list = ic_list_insert(list, value, pos);

	return list;
}


API int iotcon_list_get_nth_int(iotcon_list_h list, int index)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, 0);
	RETV_IF(NULL == list->list, 0);

	value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return 0;
	}

	return ic_value_get_int(value);
}


API bool iotcon_list_get_nth_bool(iotcon_list_h list, int index)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, false);
	RETV_IF(NULL == list->list, false);

	value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return false;
	}

	return ic_value_get_bool(value);
}


API double iotcon_list_get_nth_double(iotcon_list_h list, int index)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, 0.0);
	RETV_IF(NULL == list->list, 0.0);

	value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return 0.0;
	}

	return ic_value_get_double(value);
}


API const char* iotcon_list_get_nth_str(iotcon_list_h list, int index)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return NULL;
	}

	return ic_value_get_str(value);
}


API iotcon_list_h iotcon_list_get_nth_list(iotcon_list_h list, int index)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return NULL;
	}

	return ic_value_get_list(value);
}


API iotcon_repr_h iotcon_list_get_nth_repr(iotcon_list_h list, int index)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	value = g_list_nth_data(list->list, index);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return NULL;
	}

	return ic_value_get_repr(value);
}


static int _ic_list_del_nth_value(iotcon_list_h list, int pos, iotcon_repr_types_e value_type)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == list->list, IOTCON_ERROR_PARAM);
	RETVM_IF(value_type != list->type, IOTCON_ERROR_PARAM, "IOTCON_ERROR_PARAM(%d)",
			list->type);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	if (IOTCON_TYPE_STR == value->type) {
		ic_basic_s *real = (ic_basic_s*)value;
		free(real->val.s);
	}
	else if (IOTCON_TYPE_LIST == value->type) {
		ic_val_list_s *real = (ic_val_list_s*)value;
		iotcon_list_free(real->list);
	}
	else if (IOTCON_TYPE_REPR == value->type) {
		ic_val_repr_s *real = (ic_val_repr_s*)value;
		iotcon_repr_free(real->repr);
	}

	ic_list_remove(list, value);
	return IOTCON_ERROR_NONE;
}


API int iotcon_list_del_nth_int(iotcon_list_h list, int pos)
{
	int ret;

	ret = _ic_list_del_nth_value(list, pos, IOTCON_TYPE_INT);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_list_del_nth_int() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_bool(iotcon_list_h list, int pos)
{
	int ret;

	ret = _ic_list_del_nth_value(list, pos, IOTCON_TYPE_BOOL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ic_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_double(iotcon_list_h list, int pos)
{
	int ret;

	ret = _ic_list_del_nth_value(list, pos, IOTCON_TYPE_DOUBLE);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ic_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_str(iotcon_list_h list, int pos)
{
	int ret;

	ret = _ic_list_del_nth_value(list, pos, IOTCON_TYPE_STR);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ic_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_list(iotcon_list_h list, int pos)
{
	int ret;

	ret = _ic_list_del_nth_value(list, pos, IOTCON_TYPE_LIST);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ic_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_repr(iotcon_list_h list, int pos)
{
	int ret;

	ret = _ic_list_del_nth_value(list, pos, IOTCON_TYPE_REPR);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_ic_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_get_type(iotcon_list_h list)
{
	RETV_IF(NULL == list, IOTCON_ERROR_PARAM);

	return list->type;
}


API int iotcon_list_get_length(iotcon_list_h list)
{
	RETV_IF(NULL == list, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == list->list, IOTCON_ERROR_PARAM);

	return g_list_length(list->list);
}


int ic_list_remove(iotcon_list_h list, iotcon_value_h val)
{
	RETV_IF(NULL == list, IOTCON_ERROR_PARAM);

	list->list = g_list_remove(list->list, val);

	return IOTCON_ERROR_NONE;
}


iotcon_list_h ic_list_insert(iotcon_list_h list, iotcon_value_h value, int pos)
{
	RETV_IF(NULL == list, NULL);

	list->list = g_list_insert(list->list, value, pos);
	if (NULL == list->list) {
		ERR("g_list_insert() Fail");
		return list;
	}

	return list;
}

API void iotcon_list_foreach_int(iotcon_list_h list, iotcon_list_int_fn fn, void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETM_IF(NULL == list, "IOTCON_ERROR_PARAM(NULL == list)");
	RETM_IF(NULL == fn, "IOTCON_ERROR_PARAM(NULL == fn)");

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		(*fn)(index, real->val.i, user_data);
		index++;
		cur = next;
	}
}

API void iotcon_list_foreach_bool(iotcon_list_h list, iotcon_list_bool_fn fn, void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETM_IF(NULL == list, "IOTCON_ERROR_PARAM(NULL == list)");
	RETM_IF(NULL == fn, "IOTCON_ERROR_PARAM(NULL == fn)");

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		(*fn)(index, real->val.b, user_data);
		index++;
		cur = next;
	}
}

API void iotcon_list_foreach_double(iotcon_list_h list, iotcon_list_double_fn fn, void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETM_IF(NULL == list, "IOTCON_ERROR_PARAM(NULL == list)");
	RETM_IF(NULL == fn, "IOTCON_ERROR_PARAM(NULL == fn)");

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		(*fn)(index, real->val.d, user_data);
		index++;
		cur = next;
	}
}

API void iotcon_list_foreach_str(iotcon_list_h list, iotcon_list_str_fn fn, void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETM_IF(NULL == list, "IOTCON_ERROR_PARAM(NULL == list)");
	RETM_IF(NULL == fn, "IOTCON_ERROR_PARAM(NULL == fn)");

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		(*fn)(index, real->val.s, user_data);
		index++;
		cur = next;
	}
}

API void iotcon_list_list_foreach(iotcon_list_h list, iotcon_list_list_fn fn, void *user_data)
{
	int index = 0;
	GList *cur = NULL;
	ic_val_list_s *real = NULL;

	RETM_IF(NULL == list, "IOTCON_ERROR_PARAM(NULL == list)");
	RETM_IF(NULL == fn, "IOTCON_ERROR_PARAM(NULL == fn)");

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		(*fn)(index, real->list, user_data);
		index++;
		cur = next;
	}
}

API void iotcon_list_foreach_repr(iotcon_list_h list, iotcon_list_repr_fn fn, void *user_data)
{
	int index = 0;
	GList *cur = NULL;
	ic_val_repr_s *real = NULL;
	RETM_IF(NULL == list, "IOTCON_ERROR_PARAM(NULL == list)");
	RETM_IF(NULL == fn, "IOTCON_ERROR_PARAM(NULL == fn)");

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		(*fn)(index, real->repr, user_data);
		index++;
		cur = next;
	}
}


static iotcon_value_h _ic_list_get_nth_value(iotcon_list_h list, int index)
{
	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	return g_list_nth_data(list->list, index);
}



/*
* A general result : [1,2,3]
*/
JsonArray* ic_list_to_json(iotcon_list_h list)
{
	int i;
	int count = 0;
	JsonArray *parray = NULL;
	JsonNode *child_node = NULL;
	JsonObject *child_obj = NULL;
	JsonArray *child_array = NULL;
	iotcon_repr_h child_repr = NULL;
	iotcon_list_h child_list = NULL;
	iotcon_value_h child_value = NULL;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	count = g_list_length(list->list);
	DBG("list count(%d)", count);

	parray = json_array_new();
	for (i = 0; i < count; i++) {
		child_value = _ic_list_get_nth_value(list, i);

		int type = child_value->type;
		switch (type) {
		case IOTCON_TYPE_INT:
		case IOTCON_TYPE_BOOL:
		case IOTCON_TYPE_DOUBLE:
		case IOTCON_TYPE_STR:
		case IOTCON_TYPE_NULL:
			child_node = ic_value_to_json(child_value);
			if (NULL == child_node) {
				ERR("ic_value_to_json(child_value) Fail");
				json_array_unref(parray);
				return NULL;
			}
			json_array_add_element(parray, child_node);
			break;
		case IOTCON_TYPE_LIST:
			child_list = ic_value_get_list(child_value);
			child_array = ic_list_to_json(child_list);
			if (NULL == child_array) {
				ERR("ic_list_to_json(child_list) Fail");
				json_array_unref(parray);
				return NULL;
			}
			child_node = json_node_new(JSON_NODE_ARRAY);
			json_node_set_array(child_node, child_array);
			json_array_add_element(parray, child_node);
			break;
		case IOTCON_TYPE_REPR:
			child_repr = ic_value_get_repr(child_value);
			child_obj = ic_obj_to_json(child_repr);
			if (NULL == child_obj) {
				ERR("ic_obj_to_json(child_repr) Fail");
				json_array_unref(parray);
				return NULL;
			}
			child_node = json_node_new(JSON_NODE_OBJECT);
			json_node_set_object(child_node, child_obj);
			json_array_add_element(parray, child_node);
		}
	}

	return parray;
}

/*
* A general input : [1,2,3]
*/
iotcon_list_h ic_list_from_json(JsonArray *parray)
{
	int i;
	int count = json_array_get_length(parray);

	iotcon_list_h list = iotcon_list_new(IOTCON_TYPE_NONE);
	/*	DBG("array count(%d)", count); */

	for (i = 0; i < count; i++) {
		JsonNode *child_node = json_array_get_element(parray, i);
		if (JSON_NODE_HOLDS_NULL(child_node) || JSON_NODE_HOLDS_VALUE(child_node)) {
			iotcon_value_h value = ic_value_from_json(child_node);
			if (NULL == value) {
				ERR("ic_value_from_json() Fail");
				iotcon_list_free(list);
				return NULL;
			}

			ic_basic_s *real = (ic_basic_s*)value;
			if (IOTCON_TYPE_NONE != list->type && list->type != real->type) {
				ERR("Type matching Fail(list:%d,value:%d)", list->type, real->type);
				ic_value_free(value);
				iotcon_list_free(list);
				return NULL;
			}

			list = ic_list_insert(list, value, -1);
			list->type = real->type;
		}
		else if (JSON_NODE_HOLDS_ARRAY(child_node)) {
			if (IOTCON_TYPE_NONE != list->type && IOTCON_TYPE_LIST != list->type) {
				ERR("Type matching Fail(%d)", list->type);
				iotcon_list_free(list);
				return NULL;
			}

			JsonArray *child_array = json_node_get_array(child_node);
			iotcon_list_h parsed_list = ic_list_from_json(child_array);
			if (NULL == parsed_list) {
				ERR("ic_list_from_json() Fail(NULL == parsed_list)");
				iotcon_list_free(list);
				return NULL;
			}

			iotcon_value_h value = ic_value_new_list(parsed_list);
			if (NULL == value) {
				ERR("ic_value_new_list(%p) Fail", parsed_list);
				iotcon_list_free(parsed_list);
				iotcon_list_free(list);
				return NULL;
			}

			list = ic_list_insert(list, value, -1);
			list->type = IOTCON_TYPE_LIST;
		}
		else if (JSON_NODE_HOLDS_OBJECT(child_node)) {
			if (IOTCON_TYPE_NONE != list->type && IOTCON_TYPE_REPR != list->type) {
				ERR("Type matching Fail(%d)", list->type);
				iotcon_list_free(list);
				return NULL;
			}

			JsonObject *child_obj = json_node_get_object(child_node);
			iotcon_repr_h ret_repr = ic_obj_from_json(child_obj);
			if (NULL == ret_repr) {
				ERR("ic_obj_from_json() Fail(NULL == ret_repr)");
				iotcon_list_free(list);
				return NULL;
			}

			iotcon_value_h value = ic_value_new_repr(ret_repr);
			if (NULL == value) {
				ERR("ic_value_new_repr(%p) Fail", ret_repr);
				iotcon_repr_free(ret_repr);
				iotcon_list_free(list);
			}

			list = ic_list_insert(list, value, -1);
			list->type = IOTCON_TYPE_REPR;
		}
	}

	return list;
}

API void iotcon_list_free(iotcon_list_h list)
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
		ic_value_free(cur->data);
		cur = cur->next;
	}
	free(list);
}

static int _ic_list_clone_value(iotcon_list_h list, iotcon_list_h ret_list)
{
	int i, count;
	iotcon_value_h value, copied_value;

	count = g_list_length(list->list);
	for (i = 0; i < count; i++) {
		value = _ic_list_get_nth_value(list, i);
		if (list->type != value->type) {
			ERR("Type Mismatching(list:%d, value:%d)", list->type, value->type);
			return IOTCON_ERROR_FAIL;
		}

		copied_value = ic_value_clone(value);
		if (NULL == copied_value) {
			ERR("ic_value_clone() Fail");
			return IOTCON_ERROR_FAIL;
		}

		ic_list_insert(ret_list, copied_value, -1);
	}

	return IOTCON_ERROR_NONE;
}


static int _ic_list_clone_list(iotcon_list_h list, iotcon_list_h ret_list)
{
	int i, count;
	iotcon_list_h list_val, copied_list;

	count = g_list_length(list->list);
	for (i = 0; i < count; i++) {
		list_val = iotcon_list_get_nth_list(list, i);
		if (NULL != list_val) {
			ERR("iotcon_list_get_nth_list() Fail");
			return IOTCON_ERROR_FAIL;
		}

		copied_list = ic_list_clone(list_val);
		if (NULL == copied_list) {
			ERR("ic_list_clone() Fail");
			return IOTCON_ERROR_FAIL;
		}

		iotcon_list_insert_list(ret_list, copied_list, -1);
	}

	return IOTCON_ERROR_NONE;
}


static int _ic_list_clone_repr(iotcon_list_h list, iotcon_list_h ret_list)
{
	int i, count;
	iotcon_repr_h repr_val, copied_repr;

	count = g_list_length(list->list);
	for (i = 0; i < count; i++) {
		repr_val = iotcon_list_get_nth_repr(list, i);
		if (NULL == repr_val) {
			ERR("iotcon_list_get_nth_repr() Fail");
			return IOTCON_ERROR_FAIL;
		}

		copied_repr = iotcon_repr_clone(repr_val);
		if (NULL == copied_repr) {
			ERR("_ic_repr_clone_repr() Fail");
			return IOTCON_ERROR_FAIL;
		}

		iotcon_list_insert_repr(ret_list, copied_repr, -1);
	}

	return IOTCON_ERROR_NONE;
}


iotcon_list_h ic_list_clone(iotcon_list_h list)
{
	int ret;
	iotcon_list_h ret_list = NULL;

	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	ret_list = iotcon_list_new(list->type);
	if (NULL == ret_list) {
		ERR("iotcon_list_new(%d) Fail", list->type);
		return NULL;
	}

	switch (list->type) {
		case IOTCON_TYPE_INT:
		case IOTCON_TYPE_BOOL:
		case IOTCON_TYPE_DOUBLE:
		case IOTCON_TYPE_STR:
		case IOTCON_TYPE_NULL:
			ret = _ic_list_clone_value(list, ret_list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_ic_list_clone_value() Fail(%d)", ret);
				iotcon_list_free(ret_list);
				return NULL;
			}
			break;
		case IOTCON_TYPE_LIST:
			ret = _ic_list_clone_list(list, ret_list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_ic_list_clone_list() Fail(%d)", ret);
				iotcon_list_free(ret_list);
				return NULL;
			}
			break;
		case IOTCON_TYPE_REPR:
			ret = _ic_list_clone_repr(list, ret_list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_ic_list_clone_repr() Fail(%d)", ret);
				iotcon_list_free(ret_list);
				return NULL;
			}
			break;
		default:
			ERR("Invalid type(%d)", list->type);
			break;
	}

	return ret_list;
}
