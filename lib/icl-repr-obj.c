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

#include <glib.h>

#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "iotcon-representation.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-repr-list.h"
#include "icl-repr-value.h"
#include "icl-repr.h"
#include "icl-repr-obj.h"

int icl_obj_del_value(iotcon_repr_h repr, const char *key,
		iotcon_types_e value_type)
{
	gboolean ret = FALSE;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(repr->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup(%s) Fail", key);
		return IOTCON_ERROR_NO_DATA;
	}

	if (value_type != value->type) {
		ERR("Type matching Fail(input:%d, saved:%d)", value_type, value->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	ret = g_hash_table_remove(repr->hash_table, key);
	if (FALSE == ret) {
		ERR("g_hash_table_remove(%s) Fail", key);
		return IOTCON_ERROR_NO_DATA;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_get_int(iotcon_repr_h repr, const char *key, int *val)
{
	iotcon_value_h value;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(repr->hash_table, key);
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

API int iotcon_repr_set_int(iotcon_repr_h repr, const char *key, int val)
{
	iotcon_value_h value;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_new_int(val);
	if (NULL == value) {
		ERR("icl_value_new_int(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_del_int(iotcon_repr_h repr, const char *key)
{
	int ret;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_obj_del_value(repr, key, IOTCON_TYPE_INT);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_obj_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_repr_get_bool(iotcon_repr_h repr, const char *key, bool *val)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(repr->hash_table, key);
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

API int iotcon_repr_set_bool(iotcon_repr_h repr, const char *key, bool val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_new_bool(val);
	if (NULL == value) {
		ERR("icl_value_new_bool(%d) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_del_bool(iotcon_repr_h repr, const char *key)
{
	int ret;

	ret = icl_obj_del_value(repr, key, IOTCON_TYPE_BOOL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_obj_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_repr_get_double(iotcon_repr_h repr, const char *key, double *val)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(repr->hash_table, key);
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

API int iotcon_repr_set_double(iotcon_repr_h repr, const char *key, double val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_new_double(val);
	if (NULL == value) {
		ERR("icl_value_new_double(%f) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_del_double(iotcon_repr_h repr, const char *key)
{
	int ret;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_obj_del_value(repr, key, IOTCON_TYPE_DOUBLE);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_obj_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_repr_get_str(iotcon_repr_h repr, const char *key, char **val)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(repr->hash_table, key);
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

API int iotcon_repr_set_str(iotcon_repr_h repr, const char *key, char *val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_new_str(val);
	if (NULL == value) {
		ERR("icl_value_new_str(%s) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_del_str(iotcon_repr_h repr, const char *key)
{
	int ret;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_obj_del_value(repr, key, IOTCON_TYPE_STR);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_obj_del_value() Fail(%d)", ret);

	return ret;
}

API bool iotcon_repr_is_null(iotcon_repr_h repr, const char *key)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, false);
	RETV_IF(NULL == key, false);

	value = (iotcon_value_h) g_hash_table_lookup(repr->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return false;
	}

	real = (icl_basic_s*)value;

	return (IOTCON_TYPE_NULL == real->type) ? true : false;
}

API int iotcon_repr_set_null(iotcon_repr_h repr, const char *key)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_new_null();
	if (NULL == value) {
		ERR("icl_value_new_null() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_del_null(iotcon_repr_h repr, const char *key)
{
	int ret;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_obj_del_value(repr, key, IOTCON_TYPE_NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_obj_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_repr_get_list(iotcon_repr_h repr, const char *key, iotcon_list_h *list)
{
	iotcon_value_h value = NULL;
	icl_val_list_s *real = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(repr->hash_table, key);
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

API int iotcon_repr_set_list(iotcon_repr_h repr, const char *key, iotcon_list_h list)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_new_list(list);
	if (NULL == value) {
		ERR("icl_value_new_list() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	icl_list_inc_ref_count(list);

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_del_list(iotcon_repr_h repr, const char *key)
{
	int ret;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_obj_del_value(repr, key, IOTCON_TYPE_LIST);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_obj_del_value() Fail(%d)", ret);

	return ret;
}


API int iotcon_repr_get_repr(iotcon_repr_h src, const char *key, iotcon_repr_h *dest)
{
	icl_val_repr_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(src->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_val_repr_s*)value;
	if (IOTCON_TYPE_REPR != real->type) {
		ERR("Invalid Type(%d)", real->type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	*dest = real->repr;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_set_repr(iotcon_repr_h repr, const char *key, iotcon_repr_h val)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);

	value = icl_value_new_repr(val);
	if (NULL == value) {
		ERR("icl_value_new_repr(%p) Fail", val);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	icl_repr_inc_ref_count(val);

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_del_repr(iotcon_repr_h repr, const char *key)
{
	int ret;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_obj_del_value(repr, key, IOTCON_TYPE_REPR);
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_obj_del_value() Fail(%d)", ret);

	return ret;
}

API int iotcon_repr_get_type(iotcon_repr_h repr, const char *key, int *type)
{
	iotcon_value_h value = NULL;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == type, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(repr->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}
	*type = value->type;

	return IOTCON_ERROR_NONE;
}

int icl_obj_set_value(iotcon_repr_h repr, const char *key, iotcon_value_h value)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}
