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

#include "iotcon-types.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-list.h"
#include "icl-value.h"
#include "icl-representation.h"
#include "icl-state.h"

iotcon_state_h icl_state_ref(iotcon_state_h state)
{
	RETV_IF(NULL == state, NULL);
	RETV_IF(state->ref_count <= 0, NULL);

	state->ref_count++;

	return state;
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
	state->ref_count = 1;

	*ret_state = state;

	return IOTCON_ERROR_NONE;
}


API void iotcon_state_destroy(iotcon_state_h state)
{
	RET_IF(NULL == state);

	state->ref_count--;

	if (0 != state->ref_count)
		return;

	g_hash_table_destroy(state->hash_table);
	free(state);
}


int icl_state_del_value(iotcon_state_h state, const char *key)
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

API int iotcon_state_add_int(iotcon_state_h state, const char *key, int val)
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

API int iotcon_state_remove(iotcon_state_h state, const char *key)
{
	int ret;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_state_del_value(state, key);
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

API int iotcon_state_add_bool(iotcon_state_h state, const char *key, bool val)
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

API int iotcon_state_add_double(iotcon_state_h state, const char *key, double val)
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

API int iotcon_state_add_str(iotcon_state_h state, const char *key, char *val)
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

API int iotcon_state_is_null(iotcon_state_h state, const char *key, bool *is_null)
{
	icl_basic_s *real = NULL;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == is_null, IOTCON_ERROR_INVALID_PARAMETER);

	value = (iotcon_value_h) g_hash_table_lookup(state->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	real = (icl_basic_s*)value;
	*is_null = (IOTCON_TYPE_NULL == real->type) ? true : false;

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_add_null(iotcon_state_h state, const char *key)
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

API int iotcon_state_add_list(iotcon_state_h state, const char *key, iotcon_list_h list)
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

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
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

API int iotcon_state_add_state(iotcon_state_h state, const char *key, iotcon_state_h val)
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

	g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERROR_NONE;
}

API int iotcon_state_get_type(iotcon_state_h state, const char *key,
		iotcon_type_e *type)
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


API int iotcon_state_get_keys_count(iotcon_state_h state, unsigned int *count)
{
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == count, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state->hash_table, IOTCON_ERROR_INVALID_PARAMETER);

	*count = g_hash_table_size(state->hash_table);

	return IOTCON_ERROR_NONE;
}


API int iotcon_state_clone(iotcon_state_h state, iotcon_state_h *state_clone)
{
	int ret;

	iotcon_state_h temp = NULL;

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state_clone, IOTCON_ERROR_INVALID_PARAMETER);

	if (state->hash_table) {
		ret = iotcon_state_create(&temp);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_create() Fail(%d)", ret);
			return ret;
		}

		g_hash_table_foreach(state->hash_table, (GHFunc)icl_state_clone_foreach, temp);
	}

	*state_clone = temp;

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

