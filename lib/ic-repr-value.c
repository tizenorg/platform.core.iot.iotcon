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
#include "iotcon-representation.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-repr.h"
#include "ic-repr-list.h"
#include "ic-repr-value.h"

static iotcon_value_h _ic_value_new(int type)
{
	iotcon_value_h ret_val;

	switch (type) {
	case IOTCON_TYPE_INT:
	case IOTCON_TYPE_BOOL:
	case IOTCON_TYPE_DOUBLE:
	case IOTCON_TYPE_STR:
	case IOTCON_TYPE_NULL:
		ret_val = calloc(1, sizeof(ic_basic_s));
		break;
	case IOTCON_TYPE_LIST:
		ret_val = calloc(1, sizeof(ic_val_list_s));
		break;
	case IOTCON_TYPE_REPR:
		ret_val = calloc(1, sizeof(ic_val_repr_s));
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


iotcon_value_h ic_value_new_null()
{
	iotcon_value_h value;

	value = _ic_value_new(IOTCON_TYPE_NULL);
	if (NULL == value) {
		ERR("_ic_value_new(NULL) Fail");
		return NULL;
	}

	return value;
}

iotcon_value_h ic_value_new_int(int val)
{
	ic_basic_s *value;

	value = (ic_basic_s*)_ic_value_new(IOTCON_TYPE_INT);
	if (NULL == value) {
		ERR("_ic_value_new(INT:%d) Fail", val);
		return NULL;
	}

	value->val.i = val;

	return (iotcon_value_h)value;
}

iotcon_value_h ic_value_new_bool(bool val)
{
	ic_basic_s *value;

	value = (ic_basic_s*)_ic_value_new(IOTCON_TYPE_BOOL);
	if (NULL == value) {
		ERR("_ic_value_new(BOOL:%d) Fail", val);
		return NULL;
	}

	value->val.b = val;

	return (iotcon_value_h)value;
}

iotcon_value_h ic_value_new_double(double val)
{
	ic_basic_s *value;

	value = (ic_basic_s*)_ic_value_new(IOTCON_TYPE_DOUBLE);
	if (NULL == value) {
		ERR("_ic_value_new(DOUBLE:%f) Fail", val);
		return NULL;
	}

	value->val.d = val;

	return (iotcon_value_h)value;
}

iotcon_value_h ic_value_new_str(char *val)
{
	ic_basic_s *value;

	RETV_IF(NULL == val, NULL);

	value = (ic_basic_s*)_ic_value_new(IOTCON_TYPE_STR);
	if (NULL == value) {
		ERR("_ic_value_new(STR:%s) Fail", val);
		return NULL;
	}

	value->val.s = ic_utils_strdup(val);

	return (iotcon_value_h)value;
}


iotcon_value_h ic_value_new_list(iotcon_list_h val)
{
	ic_val_list_s *value;

	value = (ic_val_list_s*)_ic_value_new(IOTCON_TYPE_LIST);
	if (NULL == value) {
		ERR("_ic_value_new(LIST) Fail");
		return NULL;
	}

	value->list = val;

	return (iotcon_value_h)value;
}

iotcon_value_h ic_value_new_repr(iotcon_repr_h val)
{
	ic_val_repr_s *value;

	value = (ic_val_repr_s*)_ic_value_new(IOTCON_TYPE_REPR);
	if (NULL == value) {
		ERR("_ic_value_new(REPR) Fail");
		return NULL;
	}

	value->repr = val;

	return (iotcon_value_h)value;
}

int ic_value_get_int(iotcon_value_h value, int *val)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_INT != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.i;

	return IOTCON_ERROR_NONE;
}

int ic_value_get_bool(iotcon_value_h value, bool *val)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_BOOL != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.b;

	return IOTCON_ERROR_NONE;
}

int ic_value_get_double(iotcon_value_h value, double *val)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_DOUBLE != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.d;

	return IOTCON_ERROR_NONE;
}

int ic_value_get_str(iotcon_value_h value, const char **val)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_STR != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*val = real->val.s;

	return IOTCON_ERROR_NONE;
}


int ic_value_get_list(iotcon_value_h value, iotcon_list_h *list)
{
	ic_val_list_s *real = (ic_val_list_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_LIST != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*list = real->list;

	return IOTCON_ERROR_NONE;
}

int ic_value_get_repr(iotcon_value_h value, iotcon_repr_h *repr)
{
	ic_val_repr_s *real = (ic_val_repr_s*)value;

	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_TYPE_REPR != real->type, IOTCON_ERROR_INVALID_PARAMETER,
			"Invalid Type(%d)", real->type);

	*repr = real->repr;

	return IOTCON_ERROR_NONE;
}

/*
* A general result : 1
*                  : true
*                                     : 5.5
*                  : "Hello"
*/
JsonNode* ic_value_to_json(iotcon_value_h value)
{
	JsonNode *node;
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, NULL);

	if (IOTCON_TYPE_NULL == value->type)
		node = json_node_new(JSON_NODE_NULL);
	else
		node = json_node_new(JSON_NODE_VALUE);

	if (NULL == node) {
		ERR("json_node_new(%d) Fail", value->type);
		return NULL;
	}

	switch (value->type) {
	case IOTCON_TYPE_INT:
		json_node_set_int(node, real->val.i);
		break;
	case IOTCON_TYPE_BOOL:
		json_node_set_boolean(node, real->val.b);
		break;
	case IOTCON_TYPE_DOUBLE:
		json_node_set_double(node, real->val.d);
		break;
	case IOTCON_TYPE_STR:
		json_node_set_string(node, real->val.s);
		break;
	case IOTCON_TYPE_NULL:
		break;
	default:
		ERR("Invalid type(%d)", value->type);
		break;
	}

	return node;
}

/*
* A general result : 1
*                  : true
*                  : 5.5
*                  : "Hello"
*/
API iotcon_value_h ic_value_from_json(JsonNode *node)
{
	gint64 ival64;
	GType gtype = 0;
	iotcon_value_h value = NULL;

	RETV_IF(NULL == node, NULL);

	if (JSON_NODE_HOLDS_NULL(node)) {
		value = ic_value_new_null();
		if (NULL == value)
			ERR("ic_value_new_null() Fail");
		return value;
	}

	gtype = json_node_get_value_type(node);
	switch (gtype) {
		case G_TYPE_INT64:
			ival64 = json_node_get_int(node);
			if (INT_MAX < ival64 || ival64 < INT_MIN) {
				ERR("value SHOULD NOT exceeds the integer range. ival64(%lld)", ival64);
				return NULL;
			}
			value = ic_value_new_int(ival64);
			if (NULL == value)
				ERR("ic_value_new_int(%ll) Fail", ival64);
			break;
		case G_TYPE_BOOLEAN:
			value = ic_value_new_bool(json_node_get_boolean(node));
			if (NULL == value)
				ERR("ic_value_new_bool() Fail");
			break;
		case G_TYPE_DOUBLE:
			value = ic_value_new_double(json_node_get_double(node));
			if (NULL == value)
				ERR("ic_value_new_double() Fail");
			break;
		case G_TYPE_STRING:
			value = ic_value_new_str(ic_utils_strdup(json_node_get_string(node)));
			if (NULL == value)
				ERR("ic_value_new_str() Fail");
			break;
		default:
			ERR("Invalid type(%d)", gtype);
			break;
	}

	return value;
}

void ic_value_free(gpointer data)
{
	FN_CALL;
	int ret;
	const char *str;
	iotcon_value_h value;
	iotcon_list_h list;
	iotcon_repr_h repr;

	RET_IF(NULL == data);

	value = data;

	int type = value->type;
	switch (type) {
	case IOTCON_TYPE_STR:
		ret = ic_value_get_str(value, &str);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_value_get_str() Fail(%d)", ret);
			break;
		}
		free((char*)str);
	case IOTCON_TYPE_INT:
	case IOTCON_TYPE_BOOL:
	case IOTCON_TYPE_DOUBLE:
	case IOTCON_TYPE_NULL:
		break;
	case IOTCON_TYPE_LIST:
		DBG("value is list");
		ret = ic_value_get_list(value, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_value_get_list() Fail(%d)", ret);
			break;
		}
		iotcon_list_free(list);
		break;
	case IOTCON_TYPE_REPR:
		DBG("value is Repr");
		ret = ic_value_get_repr(value, &repr);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_value_get_repr() Fail(%d)", ret);
			break;
		}
		iotcon_repr_free(repr);
		break;
	default:
		ERR("Invalid type(%d)", type);
		break;
	}
	free(value);
}


iotcon_value_h ic_value_clone(iotcon_value_h src)
{
	iotcon_value_h dest = NULL;
	ic_basic_s *real = (ic_basic_s*)src;

	RETV_IF(NULL == src, NULL);

	switch (src->type) {
	case IOTCON_TYPE_INT:
		dest = ic_value_new_int(real->val.i);
		break;
	case IOTCON_TYPE_BOOL:
		dest = ic_value_new_bool(real->val.b);
		break;
	case IOTCON_TYPE_DOUBLE:
		dest = ic_value_new_double(real->val.d);
		break;
	case IOTCON_TYPE_STR:
		dest = ic_value_new_str(ic_utils_strdup(real->val.s));
		break;
	case IOTCON_TYPE_NULL:
		dest = ic_value_new_null();
		break;
	default:
		ERR("Invalid type(%d)", src->type);
		break;
	}

	if (NULL == dest)
		ERR("ic_value_new_xxx(%d) Fail", src->type);

	return dest;
}
