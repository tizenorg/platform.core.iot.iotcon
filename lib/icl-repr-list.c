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
#include "icl.h"
#include "icl-repr-obj.h"
#include "icl-repr.h"
#include "icl-repr-value.h"
#include "icl-repr-list.h"

void icl_list_inc_ref_count(iotcon_list_h val)
{
	RET_IF(NULL == val);
	RETM_IF(val->ref_count < 0, "Invalid Count(%d)", val->ref_count);

	val->ref_count++;
}

static bool _icl_list_dec_ref_count(iotcon_list_h val)
{
	bool ret;

	RETV_IF(NULL == val, false);
	RETVM_IF(val->ref_count <= 0, false, "Invalid Count(%d)", val->ref_count);

	val->ref_count--;
	if (0 == val->ref_count)
		ret = true;
	else
		ret = false;

	return ret;
}

static iotcon_list_h _icl_list_new(iotcon_types_e type)
{
	iotcon_list_h list;

	list = calloc(1, sizeof(struct ic_list_s));
	if (NULL == list) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}
	icl_list_inc_ref_count(list);
	list->type = type;

	return list;
}

API iotcon_list_h iotcon_list_new(iotcon_types_e type)
{
	if (type < IOTCON_TYPE_INT || IOTCON_TYPE_REPR < type) {
		ERR("Invalid Type(%d)", type);
		return NULL;
	}

	return _icl_list_new(type);
}


API int iotcon_list_insert_int(iotcon_list_h list, int val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_INT != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);

	value = icl_value_new_int(val);
	if (NULL == value) {
		ERR("icl_value_new_int(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_insert_bool(iotcon_list_h list, bool val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_BOOL != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);

	value = icl_value_new_bool(val);
	if (NULL == value) {
		ERR("icl_value_new_bool(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_insert_double(iotcon_list_h list, double val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_DOUBLE != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);

	value = icl_value_new_double(val);
	if (NULL == value) {
		ERR("icl_value_new_double(%f) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_insert_str(iotcon_list_h list, char *val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STR != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);

	value = icl_value_new_str(val);
	if (NULL == value) {
		ERR("icl_value_new_str(%s) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_insert_list(iotcon_list_h list, iotcon_list_h val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_LIST != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);

	value = icl_value_new_list(val);
	if (NULL == value) {
		ERR("icl_value_new_list(%p) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	icl_list_inc_ref_count(val);

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_insert_repr(iotcon_list_h list, iotcon_repr_h val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_REPR != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);

	value = icl_value_new_repr(val);
	if (NULL == value) {
		ERR("icl_value_new_repr(%p) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	icl_repr_inc_ref_count(val);

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_get_nth_int(iotcon_list_h list, int pos, int *val)
{
	int ival, ret;
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = icl_value_get_int(value, &ival);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_value_get_int() Fail(%d)", ret);
		return IOTCON_ERROR_REPRESENTATION;
	}

	*val = ival;

	return IOTCON_ERROR_NONE;
}


API int iotcon_list_get_nth_bool(iotcon_list_h list, int pos, bool *val)
{
	int ret;
	bool bval;
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = icl_value_get_bool(value, &bval);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_value_get_bool() Fail(%d)", ret);
		return IOTCON_ERROR_REPRESENTATION;
	}

	*val = bval;

	return IOTCON_ERROR_NONE;
}


API int iotcon_list_get_nth_double(iotcon_list_h list, int pos, double *val)
{
	int ret;
	double dbval;
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = icl_value_get_double(value, &dbval);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_value_get_double() Fail(%d)", ret);
		return IOTCON_ERROR_REPRESENTATION;
	}

	*val = dbval;

	return IOTCON_ERROR_NONE;
}


API int iotcon_list_get_nth_str(iotcon_list_h list, int pos, const char **val)
{
	int ret;
	const char *strval;
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = icl_value_get_str(value, &strval);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_value_get_str() Fail(%d)", ret);
		return IOTCON_ERROR_REPRESENTATION;
	}

	*val = strval;

	return IOTCON_ERROR_NONE;
}


API int iotcon_list_get_nth_list(iotcon_list_h src, int pos, iotcon_list_h *dest)
{
	int ret;
	iotcon_value_h value;
	iotcon_list_h list_val;

	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == src->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(src->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = icl_value_get_list(value, &list_val);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_value_get_list() Fail(%d)", ret);
		return IOTCON_ERROR_REPRESENTATION;
	}

	*dest = list_val;

	return IOTCON_ERROR_NONE;
}


API int iotcon_list_get_nth_repr(iotcon_list_h list, int pos, iotcon_repr_h *repr)
{
	int ret;
	iotcon_value_h value;
	iotcon_repr_h repr_val;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = icl_value_get_repr(value, &repr_val);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_value_get_list() Fail(%d)", ret);
		return IOTCON_ERROR_REPRESENTATION;
	}

	*repr = repr_val;

	return IOTCON_ERROR_NONE;
}


static int _icl_list_del_nth_value(iotcon_list_h list, int pos, iotcon_types_e value_type)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(value_type != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"IOTCON_ERROR_PARAMETER(%d)", list->type);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	if (IOTCON_TYPE_STR == value->type) {
		ic_basic_s *real = (ic_basic_s*)value;
		free(real->val.s);
	} else if (IOTCON_TYPE_LIST == value->type) {
		ic_val_list_s *real = (ic_val_list_s*)value;
		iotcon_list_free(real->list);
	} else if (IOTCON_TYPE_REPR == value->type) {
		ic_val_repr_s *real = (ic_val_repr_s*)value;
		iotcon_repr_free(real->repr);
	}

	icl_list_remove(list, value);
	return IOTCON_ERROR_NONE;
}


API int iotcon_list_del_nth_int(iotcon_list_h list, int pos)
{
	int ret;

	ret = _icl_list_del_nth_value(list, pos, IOTCON_TYPE_INT);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_list_del_nth_int() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_bool(iotcon_list_h list, int pos)
{
	int ret;

	ret = _icl_list_del_nth_value(list, pos, IOTCON_TYPE_BOOL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_icl_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_double(iotcon_list_h list, int pos)
{
	int ret;

	ret = _icl_list_del_nth_value(list, pos, IOTCON_TYPE_DOUBLE);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_icl_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_str(iotcon_list_h list, int pos)
{
	int ret;

	ret = _icl_list_del_nth_value(list, pos, IOTCON_TYPE_STR);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_icl_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_list(iotcon_list_h list, int pos)
{
	int ret;

	ret = _icl_list_del_nth_value(list, pos, IOTCON_TYPE_LIST);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_icl_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_del_nth_repr(iotcon_list_h list, int pos)
{
	int ret;

	ret = _icl_list_del_nth_value(list, pos, IOTCON_TYPE_REPR);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_icl_list_del_nth_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_list_get_type(iotcon_list_h list, int *type)
{
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == type, IOTCON_ERROR_INVALID_PARAMETER);

	*type = list->type;

	return IOTCON_ERROR_NONE;
}


API unsigned int iotcon_list_get_length(iotcon_list_h list)
{
	RETV_IF(NULL == list, 0);
	RETV_IF(NULL == list->list, 0);

	return g_list_length(list->list);
}


int icl_list_remove(iotcon_list_h list, iotcon_value_h val)
{
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	list->list = g_list_remove(list->list, val);

	return IOTCON_ERROR_NONE;
}


int icl_list_insert(iotcon_list_h list, iotcon_value_h value, int pos)
{
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	list->list = g_list_insert(list->list, value, pos);

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_int(iotcon_list_h list, iotcon_list_int_fn fn,
		void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_INT != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == fn(index, real->val.i, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_bool(iotcon_list_h list, iotcon_list_bool_fn fn,
		void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_BOOL != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == fn(index, real->val.b, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_double(iotcon_list_h list, iotcon_list_double_fn fn,
		void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_DOUBLE != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == fn(index, real->val.d, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_str(iotcon_list_h list, iotcon_list_str_fn fn,
		void *user_data)
{
	GList *cur;
	int index = 0;
	ic_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STR != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == fn(index, real->val.s, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_list(iotcon_list_h list, iotcon_list_list_fn fn,
		void *user_data)
{
	int index = 0;
	GList *cur = NULL;
	ic_val_list_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_LIST != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == fn(index, real->list, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_repr(iotcon_list_h list, iotcon_list_repr_fn fn, void *user_data)
{
	int index = 0;
	GList *cur = NULL;
	ic_val_repr_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_REPR != list->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);;

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == fn(index, real->repr, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}


static iotcon_value_h _icl_list_get_nth_value(iotcon_list_h list, int pos)
{
	RETV_IF(NULL == list, NULL);
	RETV_IF(NULL == list->list, NULL);

	return g_list_nth_data(list->list, pos);
}



/*
 * A general result : [1,2,3]
 */
JsonArray* icl_list_to_json(iotcon_list_h list)
{
	int i, ret, count;
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

	parray = json_array_new();
	for (i = 0; i < count; i++) {
		child_value = _icl_list_get_nth_value(list, i);

		int type = child_value->type;
		switch (type) {
		case IOTCON_TYPE_INT:
		case IOTCON_TYPE_BOOL:
		case IOTCON_TYPE_DOUBLE:
		case IOTCON_TYPE_STR:
		case IOTCON_TYPE_NULL:
			child_node = icl_value_to_json(child_value);
			if (NULL == child_node) {
				ERR("icl_value_to_json(child_value) Fail");
				json_array_unref(parray);
				return NULL;
			}
			json_array_add_element(parray, child_node);
			break;
		case IOTCON_TYPE_LIST:
			ret = icl_value_get_list(child_value, &child_list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_value_get_list() Fail(%d)", ret);
				json_array_unref(parray);
				return NULL;
			}

			child_array = icl_list_to_json(child_list);
			if (NULL == child_array) {
				ERR("icl_list_to_json(child_list) Fail");
				json_array_unref(parray);
				return NULL;
			}
			child_node = json_node_new(JSON_NODE_ARRAY);
			json_node_set_array(child_node, child_array);
			json_array_add_element(parray, child_node);
			break;
		case IOTCON_TYPE_REPR:
			ret = icl_value_get_repr(child_value, &child_repr);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_value_get_repr() Fail(%d)", ret);
				json_array_unref(parray);
				return NULL;
			}

			child_obj = icl_obj_to_json(child_repr);
			if (NULL == child_obj) {
				ERR("icl_obj_to_json(child_repr) Fail");
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
iotcon_list_h icl_list_from_json(JsonArray *parray)
{
	int i, ret;
	int count = json_array_get_length(parray);

	iotcon_list_h list = _icl_list_new(IOTCON_TYPE_NONE);
	if (NULL == list) {
		ERR("_icl_list_new() Fail");
		return NULL;
	}

	for (i = 0; i < count; i++) {
		JsonNode *child_node = json_array_get_element(parray, i);
		if (JSON_NODE_HOLDS_NULL(child_node) || JSON_NODE_HOLDS_VALUE(child_node)) {
			iotcon_value_h value = icl_value_from_json(child_node);
			if (NULL == value) {
				ERR("icl_value_from_json() Fail");
				iotcon_list_free(list);
				return NULL;
			}

			ic_basic_s *real = (ic_basic_s*)value;
			if (IOTCON_TYPE_NONE != list->type && list->type != real->type) {
				ERR("Type matching Fail(list:%d,value:%d)", list->type, real->type);
				icl_value_free(value);
				iotcon_list_free(list);
				return NULL;
			}

			ret = icl_list_insert(list, value, -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_list_insert() Fail(%d)", ret);
				icl_value_free(value);
				iotcon_list_free(list);
				return NULL;
			}
			list->type = real->type;
		} else if (JSON_NODE_HOLDS_ARRAY(child_node)) {
			if (IOTCON_TYPE_NONE != list->type && IOTCON_TYPE_LIST != list->type) {
				ERR("Type matching Fail(%d)", list->type);
				iotcon_list_free(list);
				return NULL;
			}

			JsonArray *child_array = json_node_get_array(child_node);
			iotcon_list_h parsed_list = icl_list_from_json(child_array);
			if (NULL == parsed_list) {
				ERR("icl_list_from_json() Fail(NULL == parsed_list)");
				iotcon_list_free(list);
				return NULL;
			}

			iotcon_value_h value = icl_value_new_list(parsed_list);
			if (NULL == value) {
				ERR("icl_value_new_list(%p) Fail", parsed_list);
				iotcon_list_free(parsed_list);
				iotcon_list_free(list);
				return NULL;
			}

			ret = icl_list_insert(list, value, -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_list_insert() Fail(%d)", ret);
				iotcon_list_free(parsed_list);
				iotcon_list_free(list);
				return NULL;
			}
			list->type = IOTCON_TYPE_LIST;
		} else if (JSON_NODE_HOLDS_OBJECT(child_node)) {
			if (IOTCON_TYPE_NONE != list->type && IOTCON_TYPE_REPR != list->type) {
				ERR("Type matching Fail(%d)", list->type);
				iotcon_list_free(list);
				return NULL;
			}

			JsonObject *child_obj = json_node_get_object(child_node);
			iotcon_repr_h ret_repr = icl_obj_from_json(child_obj);
			if (NULL == ret_repr) {
				ERR("icl_obj_from_json() Fail(NULL == ret_repr)");
				iotcon_list_free(list);
				return NULL;
			}

			iotcon_value_h value = icl_value_new_repr(ret_repr);
			if (NULL == value) {
				ERR("icl_value_new_repr(%p) Fail", ret_repr);
				iotcon_repr_free(ret_repr);
				iotcon_list_free(list);
				return NULL;
			}

			ret = icl_list_insert(list, value, -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_list_insert() Fail(%d)", ret);
				iotcon_repr_free(ret_repr);
				iotcon_list_free(list);
				return NULL;
			}
			list->type = IOTCON_TYPE_REPR;
		}
	}

	return list;
}

API void iotcon_list_free(iotcon_list_h list)
{
	FN_CALL;
	GList *cur = NULL;

	RET_IF(NULL == list);

	if (false == _icl_list_dec_ref_count(list))
		return;

	cur = list->list;
	while (cur) {
		icl_value_free(cur->data);
		cur = cur->next;
	}
	free(list);
}

static int _icl_list_clone_value(iotcon_list_h list, iotcon_list_h ret_list)
{
	int i, ret, count;
	iotcon_value_h value, copied_value;

	count = g_list_length(list->list);
	for (i = 0; i < count; i++) {
		value = _icl_list_get_nth_value(list, i);
		if (NULL == value) {
			ERR("_icl_list_get_nth_value() Fail");
			return IOTCON_ERROR_INVALID_PARAMETER;
		}
		if (list->type != value->type) {
			ERR("Type Mismatching(list:%d, value:%d)", list->type, value->type);
			return IOTCON_ERROR_INVALID_TYPE;
		}

		copied_value = icl_value_clone(value);
		if (NULL == copied_value) {
			ERR("icl_value_clone() Fail");
			return IOTCON_ERROR_REPRESENTATION;
		}

		ret = icl_list_insert(ret_list, copied_value, -1);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_list_insert() Fail");
			icl_value_free(copied_value);
			return IOTCON_ERROR_REPRESENTATION;
		}
	}

	return IOTCON_ERROR_NONE;
}


static int _icl_list_clone_list(iotcon_list_h list, iotcon_list_h ret_list)
{
	int i, ret, count;

	iotcon_value_h value;
	iotcon_list_h list_val, copied_list;

	count = g_list_length(list->list);
	for (i = 0; i < count; i++) {
		ret = iotcon_list_get_nth_list(list, i, &list_val);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_get_nth_list() Fail(%d)", ret);
			return IOTCON_ERROR_REPRESENTATION;
		}

		copied_list = icl_list_clone(list_val);
		if (NULL == copied_list) {
			ERR("icl_list_clone() Fail");
			return IOTCON_ERROR_REPRESENTATION;
		}

		value = icl_value_new_list(copied_list);
		if (NULL == value) {
			ERR("icl_value_new_list(%p) Fail", copied_list);
			iotcon_list_free(copied_list);
			return IOTCON_ERROR_REPRESENTATION;
		}

		ret = icl_list_insert(ret_list, value, -1);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_list_insert(%d) Fail", ret);
			icl_value_free(value);
			return IOTCON_ERROR_REPRESENTATION;
		}
	}

	return IOTCON_ERROR_NONE;
}


static int _icl_list_clone_repr(iotcon_list_h list, iotcon_list_h ret_list)
{
	int i, ret, count;
	iotcon_value_h value;
	iotcon_repr_h repr_val, copied_repr;

	count = g_list_length(list->list);
	for (i = 0; i < count; i++) {
		ret = iotcon_list_get_nth_repr(list, i, &repr_val);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_get_nth_repr() Fail");
			return IOTCON_ERROR_REPRESENTATION;
		}

		copied_repr = iotcon_repr_clone(repr_val);
		if (NULL == copied_repr) {
			ERR("_ic_repr_clone_repr() Fail");
			return IOTCON_ERROR_REPRESENTATION;
		}

		value = icl_value_new_repr(copied_repr);
		if (NULL == value) {
			ERR("icl_value_new_repr(%p) Fail", copied_repr);
			iotcon_repr_free(copied_repr);
			return IOTCON_ERROR_REPRESENTATION;
		}

		ret = icl_list_insert(ret_list, value, -1);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_list_insert(%d) Fail", ret);
			icl_value_free(value);
			return IOTCON_ERROR_REPRESENTATION;
		}
	}

	return IOTCON_ERROR_NONE;
}


iotcon_list_h icl_list_clone(iotcon_list_h list)
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
		ret = _icl_list_clone_value(list, ret_list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_list_clone_value() Fail(%d)", ret);
			iotcon_list_free(ret_list);
			return NULL;
		}
		break;
	case IOTCON_TYPE_LIST:
		ret = _icl_list_clone_list(list, ret_list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_list_clone_list() Fail(%d)", ret);
			iotcon_list_free(ret_list);
			return NULL;
		}
		break;
	case IOTCON_TYPE_REPR:
		ret = _icl_list_clone_repr(list, ret_list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_list_clone_repr() Fail(%d)", ret);
			iotcon_list_free(ret_list);
			return NULL;
		}
		break;
	default:
		ERR("Invalid type(%d)", list->type);
		iotcon_list_free(ret_list);
		return NULL;
	}

	return ret_list;
}
