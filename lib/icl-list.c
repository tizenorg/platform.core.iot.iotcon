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

#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "iotcon-representation.h"
#include "icl.h"
#include "icl-state.h"
#include "icl-representation.h"
#include "icl-value.h"
#include "icl-list.h"

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

static int _icl_list_create(iotcon_types_e type, iotcon_list_h *ret_list)
{
	iotcon_list_h list;

	list = calloc(1, sizeof(struct icl_list_s));
	if (NULL == list) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	icl_list_inc_ref_count(list);
	list->type = type;

	*ret_list = list;

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_create(iotcon_types_e type, iotcon_list_h *ret_list)
{
	int ret;
	iotcon_list_h list;

	RETV_IF(NULL == ret_list, IOTCON_ERROR_INVALID_PARAMETER);

	if (type < IOTCON_TYPE_INT || IOTCON_TYPE_STATE < type) {
		ERR("Invalid Type(%d)", type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	ret = _icl_list_create(type, &list);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icl_list_create() Fail");
		return ret;
	}

	*ret_list = list;

	return IOTCON_ERROR_NONE;
}


API int iotcon_list_add_int(iotcon_list_h list, int val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_INT != list->type, IOTCON_ERROR_INVALID_TYPE, "Invalid Type(%d)",
			list->type);

	value = icl_value_create_int(val);
	if (NULL == value) {
		ERR("icl_value_create_int(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_add_bool(iotcon_list_h list, bool val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_BOOL != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);

	value = icl_value_create_bool(val);
	if (NULL == value) {
		ERR("icl_value_create_bool(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_add_double(iotcon_list_h list, double val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_DOUBLE != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);

	value = icl_value_create_double(val);
	if (NULL == value) {
		ERR("icl_value_create_double(%f) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_add_str(iotcon_list_h list, char *val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STR != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);

	value = icl_value_create_str(val);
	if (NULL == value) {
		ERR("icl_value_create_str(%s) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_add_list(iotcon_list_h list, iotcon_list_h val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_LIST != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);

	value = icl_value_create_list(val);
	if (NULL == value) {
		ERR("icl_value_create_list(%p) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	icl_list_inc_ref_count(val);

	return icl_list_insert(list, value, pos);
}


API int iotcon_list_add_state(iotcon_list_h list, iotcon_state_h val, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STATE != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);

	value = icl_value_create_state(val);
	if (NULL == value) {
		ERR("icl_value_create_state(%p) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	icl_state_inc_ref_count(val);

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


API int iotcon_list_get_nth_str(iotcon_list_h list, int pos, char **val)
{
	int ret;
	char *strval;
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


API int iotcon_list_get_nth_state(iotcon_list_h list, int pos, iotcon_state_h *state)
{
	int ret;
	iotcon_value_h value;
	iotcon_state_h state_val;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = icl_value_get_state(value, &state_val);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_value_get_state() Fail(%d)", ret);
		return IOTCON_ERROR_REPRESENTATION;
	}

	*state = state_val;

	return IOTCON_ERROR_NONE;
}


static int _icl_list_remove_nth_value(iotcon_list_h list, int pos)
{
	iotcon_value_h value;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_list_nth_data(list->list, pos);
	if (NULL == value) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	if (IOTCON_TYPE_STR == value->type) {
		icl_basic_s *real = (icl_basic_s*)value;
		free(real->val.s);
	} else if (IOTCON_TYPE_LIST == value->type) {
		icl_val_list_s *real = (icl_val_list_s*)value;
		iotcon_list_destroy(real->list);
	} else if (IOTCON_TYPE_STATE == value->type) {
		icl_val_state_s *real = (icl_val_state_s*)value;
		iotcon_state_destroy(real->state);
	}

	icl_list_remove(list, value);
	return IOTCON_ERROR_NONE;
}


API int iotcon_list_remove_nth(iotcon_list_h list, int pos)
{
	int ret;

	ret = _icl_list_remove_nth_value(list, pos);
	if (IOTCON_ERROR_NONE != ret)
		ERR("_icl_list_remove_nth_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_list_get_type(iotcon_list_h list, iotcon_types_e *type)
{
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == type, IOTCON_ERROR_INVALID_PARAMETER);

	*type = list->type;

	return IOTCON_ERROR_NONE;
}


API int iotcon_list_get_length(iotcon_list_h list, unsigned int *length)
{
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list->list, IOTCON_ERROR_INVALID_PARAMETER);

	*length = g_list_length(list->list);

	return IOTCON_ERROR_NONE;
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

API int iotcon_list_foreach_int(iotcon_list_h list, iotcon_list_int_cb cb,
		void *user_data)
{
	GList *cur;
	int index = 0;
	icl_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_INT != list->type, IOTCON_ERROR_INVALID_TYPE, "Invalid Type(%d)",
			list->type);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == cb(index, real->val.i, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_bool(iotcon_list_h list, iotcon_list_bool_cb cb,
		void *user_data)
{
	GList *cur;
	int index = 0;
	icl_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_BOOL != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == cb(index, real->val.b, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_double(iotcon_list_h list, iotcon_list_double_cb cb,
		void *user_data)
{
	GList *cur;
	int index = 0;
	icl_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_DOUBLE != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == cb(index, real->val.d, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_str(iotcon_list_h list, iotcon_list_str_cb cb,
		void *user_data)
{
	GList *cur;
	int index = 0;
	icl_basic_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STR != list->type, IOTCON_ERROR_INVALID_TYPE, "Invalid Type(%d)",
			list->type);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == cb(index, real->val.s, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_list(iotcon_list_h list, iotcon_list_list_cb cb,
		void *user_data)
{
	int index = 0;
	GList *cur = NULL;
	icl_val_list_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_LIST != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == cb(index, real->list, user_data))
			break;
		index++;
		cur = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_list_foreach_state(iotcon_list_h list, iotcon_list_state_cb cb,
		void *user_data)
{
	int index = 0;
	GList *cur = NULL;
	icl_val_state_s *real = NULL;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STATE != list->type, IOTCON_ERROR_INVALID_TYPE,
			"Invalid Type(%d)", list->type);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);;

	cur = list->list;
	while (cur) {
		GList *next = cur->next;
		real = cur->data;
		if (IOTCON_FUNC_STOP == cb(index, real->state, user_data))
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


API void iotcon_list_destroy(iotcon_list_h list)
{
	FN_CALL;
	GList *cur = NULL;

	RET_IF(NULL == list);

	if (false == _icl_list_dec_ref_count(list))
		return;

	cur = list->list;
	while (cur) {
		icl_value_destroy(cur->data);
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
			icl_value_destroy(copied_value);
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

		value = icl_value_create_list(copied_list);
		if (NULL == value) {
			ERR("icl_value_create_list(%p) Fail", copied_list);
			iotcon_list_destroy(copied_list);
			return IOTCON_ERROR_REPRESENTATION;
		}

		ret = icl_list_insert(ret_list, value, -1);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_list_insert(%d) Fail", ret);
			icl_value_destroy(value);
			return IOTCON_ERROR_REPRESENTATION;
		}
	}

	return IOTCON_ERROR_NONE;
}


static int _icl_list_clone_state(iotcon_list_h list, iotcon_list_h ret_list)
{
	int i, ret, count;
	iotcon_value_h value;
	iotcon_state_h state_val;
	iotcon_state_h copied_state = NULL;

	count = g_list_length(list->list);
	for (i = 0; i < count; i++) {
		ret = iotcon_list_get_nth_state(list, i, &state_val);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_get_nth_state() Fail");
			return IOTCON_ERROR_REPRESENTATION;
		}


		if (state_val->hash_table) {
			g_hash_table_foreach(state_val->hash_table, (GHFunc)icl_state_clone_foreach,
					copied_state);
		}

		value = icl_value_create_state(copied_state);
		if (NULL == value) {
			ERR("icl_value_create_state(%p) Fail", copied_state);
			iotcon_state_destroy(copied_state);
			return IOTCON_ERROR_REPRESENTATION;
		}

		ret = icl_list_insert(ret_list, value, -1);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_list_insert(%d) Fail", ret);
			icl_value_destroy(value);
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

	ret = iotcon_list_create(list->type, &ret_list);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_create(%d) Fail(%d)", list->type, ret);
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
			iotcon_list_destroy(ret_list);
			return NULL;
		}
		break;
	case IOTCON_TYPE_LIST:
		ret = _icl_list_clone_list(list, ret_list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_list_clone_list() Fail(%d)", ret);
			iotcon_list_destroy(ret_list);
			return NULL;
		}
		break;
	case IOTCON_TYPE_STATE:
		ret = _icl_list_clone_state(list, ret_list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_list_clone_state() Fail(%d)", ret);
			iotcon_list_destroy(ret_list);
			return NULL;
		}
		break;
	default:
		ERR("Invalid type(%d)", list->type);
		iotcon_list_destroy(ret_list);
		return NULL;
	}

	return ret_list;
}
