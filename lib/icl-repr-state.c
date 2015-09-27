/* Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <glib.h>

#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "iotcon-representation.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-repr-list.h"
#include "icl-repr-value.h"
#include "icl-repr.h"
#include "icl-repr-state.h"

void icl_state_inc_ref_count(iotcon_state_h val)
{
	RET_IF(NULL == val);
	RETM_IF(val->ref_count < 0, "Invalid Count(%d)", val->ref_count);

	val->ref_count++;
}


int icl_state_dec_ref_count(iotcon_state_h val)
{
	RETV_IF(NULL == val, -1);
	RETVM_IF(val->ref_count <= 0, 0, "Invalid Count(%d)", val->ref_count);

	val->ref_count--;

	return val->ref_count;
}


API int iotcon_state_create(iotcon_state_h *ret_state)
{
	errno = 0;
	iotcon_state_h state;

	RETV_IF(NULL == ret_state, IOTCON_ERROR_INVALID_PARAMETER);

	state = calloc(1, sizeof(struct icl_state_s));
	if (NULL == state) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	state->hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, free,
			icl_value_destroy);
	icl_state_inc_ref_count(state);

	*ret_state = state;

	return IOTCON_ERROR_NONE;
}


API void iotcon_state_destroy(iotcon_state_h state)
{
	RET_IF(NULL == state);

	if (0 == icl_state_dec_ref_count(state)) {
		g_hash_table_destroy(state->hash_table);
		free(state);
	}
}


int icl_state_del_value(iotcon_state_h state, const char *key, iotcon_types_e value_type)
{
	gboolean ret = FALSE;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup(%s) Fail", key);
		return IOTCON_ERROR_NO_DATA;
	}

	if (value_type != value->type) {
		ERR("Type matching Fail(input:%d, saved:%d)", value_type, value->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	ret = g_hash_table_remove(state->hash_table, key);
	if (FALSE == ret) {
		ERR("g_hash_table_remove(%s) Fail", key);
		return IOTCON_ERROR_NO_DATA;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_get_int(iotcon_state_h state, const char *key, int *val)
{
	iotcon_value_h value;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	icl_basic_s *real = (icl_basic_s*)value;
	if (IOTCON_TYPE_INT != real->type) {
		ERR("Invalid Type(%d)", real->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	*val = real->val.i;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_set_int(iotcon_state_h state, const char *key, int val)
{
	iotcon_value_h value;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_create_int(val);
	if (NULL == value) {
		ERR("icl_value_create_int(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_del_int(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key, IOTCON_TYPE_INT);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_state_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_state_get_bool(iotcon_state_h state, const char *key, bool *val)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_basic_s*)value;
	if (IOTCON_TYPE_BOOL != real->type) {
		ERR("Invalid Type(%d)", real->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	*val = real->val.b;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_set_bool(iotcon_state_h state, const char *key, bool val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_create_bool(val);
	if (NULL == value) {
		ERR("icl_value_create_bool(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_del_bool(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key, IOTCON_TYPE_BOOL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_state_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_state_get_double(iotcon_state_h state, const char *key, double *val)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_basic_s*)value;
	if (IOTCON_TYPE_DOUBLE != real->type) {
		ERR("Invalid Type(%d)", real->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	*val = real->val.d;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_set_double(iotcon_state_h state, const char *key, double val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_create_double(val);
	if (NULL == value) {
		ERR("icl_value_create_double(%f) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_del_double(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key, IOTCON_TYPE_DOUBLE);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_state_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_state_get_str(iotcon_state_h state, const char *key, char **val)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_basic_s*)value;
	if (IOTCON_TYPE_STR != real->type) {
		ERR("Invalid Type(%d)", real->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	*val = real->val.s;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_set_str(iotcon_state_h state, const char *key, char *val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_create_str(val);
	if (NULL == value) {
		ERR("icl_value_create_str(%s) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_del_str(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key, IOTCON_TYPE_STR);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_state_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_state_is_null(iotcon_state_h state, const char *key, bool *is_null)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = (iotcon_value_h) g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_basic_s*)value;
	*is_null = (IOTCON_TYPE_NULL == real->type) ? true : false;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_set_null(iotcon_state_h state, const char *key)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_create_null();
	if (NULL == value) {
		ERR("icl_value_create_null() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_del_null(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key, IOTCON_TYPE_NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_state_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_state_get_list(iotcon_state_h state, const char *key, iotcon_list_h *list)
{
	iotcon_value_h value = NULL;
	icl_val_list_s *real = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_val_list_s*)value;
	if (IOTCON_TYPE_LIST != real->type) {
		ERR("Invalid Type(%d)", real->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	*list = real->list;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_set_list(iotcon_state_h state, const char *key, iotcon_list_h list)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_create_list(list);
	if (NULL == value) {
		ERR("icl_value_create_list() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	icl_list_inc_ref_count(list);

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_del_list(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key, IOTCON_TYPE_LIST);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_state_del_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_state_get_state(iotcon_state_h src, const char *key, iotcon_state_h *dest)
{
	icl_val_state_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(src->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_val_state_s*)value;
	if (IOTCON_TYPE_STATE != real->type) {
		ERR("Invalid Type(%d)", real->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	*dest = real->state;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_set_state(iotcon_state_h state, const char *key, iotcon_state_h val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_create_state(val);
	if (NULL == value) {
		ERR("icl_value_create_state(%p) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	icl_state_inc_ref_count(val);

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_del_state(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key, IOTCON_TYPE_STATE);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_state_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_state_get_type(iotcon_state_h state, const char *key, int *type)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == type, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}
	*type = value->type;

	return IOTCON_ERROR_NONE;
}

int icl_state_set_value(iotcon_state_h state, const char *key, iotcon_value_h value)
{
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}
