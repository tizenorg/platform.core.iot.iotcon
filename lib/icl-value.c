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
#include "iotcon-representation.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-representation.h"
#include "icl-list.h"
#include "icl-value.h"

static iotcon_value_h _icl_value_create(int type)
{
	iotcon_value_h ret_val;

	switch (type) {
	case IOTCON_TYPE_INT:
	case IOTCON_TYPE_BOOL:
	case IOTCON_TYPE_DOUBLE:
	case IOTCON_TYPE_STR:
	case IOTCON_TYPE_NULL:
		ret_val = calloc(1, sizeof(icl_basic_s));
		break;
	case IOTCON_TYPE_LIST:
		ret_val = calloc(1, sizeof(icl_val_list_s));
		break;
	case IOTCON_TYPE_STATE:
		ret_val = calloc(1, sizeof(icl_val_state_s));
		break;
	default:
		ERR("Invalid Type(%d)", type);
		return NULL;
	}

	if (NULL == ret_val) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	ret_val->type = type;

	return ret_val;
}


iotcon_value_h icl_value_create_null()
{
	iotcon_value_h value;

	value = _icl_value_create(IOTCON_TYPE_NULL);
	if (NULL == value) {
		ERR("_icl_value_create(NULL) Fail");
		return NULL;
	}

	return value;
}

iotcon_value_h icl_value_create_int(int val)
{
	icl_basic_s *value;

	value = (icl_basic_s*)_icl_value_create(IOTCON_TYPE_INT);
	if (NULL == value) {
		ERR("_icl_value_create(INT:%d) Fail", val);
		return NULL;
	}

	value->val.i = val;

	return (iotcon_value_h)value;
}

iotcon_value_h icl_value_create_bool(bool val)
{
	icl_basic_s *value;

	value = (icl_basic_s*)_icl_value_create(IOTCON_TYPE_BOOL);
	if (NULL == value) {
		ERR("_icl_value_create(BOOL:%d) Fail", val);
		return NULL;
	}

	value->val.b = val;

	return (iotcon_value_h)value;
}

iotcon_value_h icl_value_create_double(double val)
{
	icl_basic_s *value;

	value = (icl_basic_s*)_icl_value_create(IOTCON_TYPE_DOUBLE);
	if (NULL == value) {
		ERR("_icl_value_create(DOUBLE:%f) Fail", val);
		return NULL;
	}

	value->val.d = val;

	return (iotcon_value_h)value;
}

iotcon_value_h icl_value_create_str(const char *val)
{
	icl_basic_s *value;

	RETV_IF(NULL == val, NULL);

	value = (icl_basic_s*)_icl_value_create(IOTCON_TYPE_STR);
	if (NULL == value) {
		ERR("_icl_value_create(STR:%s) Fail", val);
		return NULL;
	}

	value->val.s = ic_utils_strdup(val);

	return (iotcon_value_h)value;
}


iotcon_value_h icl_value_create_list(iotcon_list_h val)
{
	icl_val_list_s *value;

	value = (icl_val_list_s*)_icl_value_create(IOTCON_TYPE_LIST);
	if (NULL == value) {
		ERR("_icl_value_create(LIST) Fail");
		return NULL;
	}

	value->list = val;

	return (iotcon_value_h)value;
}

iotcon_value_h icl_value_create_state(iotcon_state_h val)
{
	icl_val_state_s *value;

	value = (icl_val_state_s*)_icl_value_create(IOTCON_TYPE_STATE);
	if (NULL == value) {
		ERR("_icl_value_create(state) Fail");
		return NULL;
	}

	value->state = val;

	return (iotcon_value_h)value;
}

int icl_value_get_int(iotcon_value_h value, int *val)
{
	icl_basic_s *real = (icl_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_INT != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.i;

	return IOTCON_ERROR_NONE;
}

int icl_value_get_bool(iotcon_value_h value, bool *val)
{
	icl_basic_s *real = (icl_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_BOOL != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.b;

	return IOTCON_ERROR_NONE;
}

int icl_value_get_double(iotcon_value_h value, double *val)
{
	icl_basic_s *real = (icl_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_DOUBLE != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.d;

	return IOTCON_ERROR_NONE;
}

int icl_value_get_str(iotcon_value_h value, char **val)
{
	icl_basic_s *real = (icl_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STR != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.s;

	return IOTCON_ERROR_NONE;
}


int icl_value_get_list(iotcon_value_h value, iotcon_list_h *list)
{
	icl_val_list_s *real = (icl_val_list_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_LIST != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*list = real->list;

	return IOTCON_ERROR_NONE;
}

int icl_value_get_state(iotcon_value_h value, iotcon_state_h *state)
{
	icl_val_state_s *real = (icl_val_state_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STATE != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*state = real->state;

	return IOTCON_ERROR_NONE;
}


void icl_value_destroy(gpointer data)
{
	int ret;
	iotcon_value_h value;
	iotcon_list_h list;
	iotcon_state_h state;

	RET_IF(NULL == data);

	value = data;

	int type = value->type;
	switch (type) {
	case IOTCON_TYPE_STR:
		free(((icl_basic_s*)value)->val.s);
	case IOTCON_TYPE_INT:
	case IOTCON_TYPE_BOOL:
	case IOTCON_TYPE_DOUBLE:
	case IOTCON_TYPE_NULL:
		break;
	case IOTCON_TYPE_LIST:
		DBG("value is list");
		ret = icl_value_get_list(value, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_value_get_list() Fail(%d)", ret);
			break;
		}
		iotcon_list_destroy(list);
		break;
	case IOTCON_TYPE_STATE:
		DBG("value is Repr");
		ret = icl_value_get_state(value, &state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_value_get_state() Fail(%d)", ret);
			break;
		}
		iotcon_state_destroy(state);
		break;
	default:
		ERR("Invalid type(%d)", type);
		break;
	}
	free(value);
}


iotcon_value_h icl_value_clone(iotcon_value_h src)
{
	iotcon_value_h dest = NULL;
	icl_basic_s *real = (icl_basic_s*)src;

	RETV_IF(NULL == src, NULL);

	switch (src->type) {
	case IOTCON_TYPE_INT:
		dest = icl_value_create_int(real->val.i);
		break;
	case IOTCON_TYPE_BOOL:
		dest = icl_value_create_bool(real->val.b);
		break;
	case IOTCON_TYPE_DOUBLE:
		dest = icl_value_create_double(real->val.d);
		break;
	case IOTCON_TYPE_STR:
		dest = icl_value_create_str(ic_utils_strdup(real->val.s));
		break;
	case IOTCON_TYPE_NULL:
		dest = icl_value_create_null();
		break;
	default:
		ERR("Invalid type(%d)", src->type);
		break;
	}

	if (NULL == dest)
		ERR("ic_value_create_xxx(%d) Fail", src->type);

	return dest;
}
