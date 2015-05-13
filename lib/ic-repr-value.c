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
#include "ic-utils.h"
#include "ic-repr.h"
#include "ic-repr-list.h"
#include "ic-repr-value.h"

iotcon_value_h ic_value_new(int type)
{
	iotcon_value_h ret_val;
	errno = 0;

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

API int iotcon_value_get_type(iotcon_value_h value)
{
	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	return value->type;
}

API int iotcon_value_get_int(iotcon_value_h value)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_INT != real->type, IOTCON_ERR_PARAM, "IOTCON_ERR_PARAM(%d)",
			real->type);

	return real->val.i;
}

int ic_value_set_int(iotcon_value_h value, int ival)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_INT != real->type, IOTCON_ERR_PARAM, "IOTCON_ERR_PARAM(%d)",
			real->type);

	real->val.i = ival;

	return IOTCON_ERR_NONE;
}

API bool iotcon_value_get_bool(iotcon_value_h value)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_BOOL != real->type, false, "IOTCON_ERR_PARAM(%d)", real->type);

	return real->val.b;
}

int ic_value_set_bool(iotcon_value_h value, bool bval)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_BOOL != real->type, IOTCON_ERR_PARAM, "IOTCON_ERR_PARAM(%d)",
			real->type);

	real->val.b = bval;

	return IOTCON_ERR_NONE;
}

API double iotcon_value_get_double(iotcon_value_h value)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_DOUBLE != real->type, 0, "IOTCON_ERR_PARAM(%d)", real->type);

	return real->val.d;
}

int ic_value_set_double(iotcon_value_h value, double dbval)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_DOUBLE != real->type, IOTCON_ERR_PARAM, "IOTCON_ERR_PARAM(%d)",
			real->type);

	real->val.d = dbval;

	return IOTCON_ERR_NONE;
}

API char* iotcon_value_get_str(iotcon_value_h value)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, NULL);
	RETVM_IF(IOTCON_TYPE_STR != real->type, NULL, "IOTCON_ERR_PARAM(%d)", real->type);

	return real->val.s;
}

int ic_value_set_str(iotcon_value_h value, const char *strval)
{
	ic_basic_s *real = (ic_basic_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_STR != real->type, IOTCON_ERR_PARAM, "IOTCON_ERR_PARAM(%d)",
			real->type);

	real->val.s = ic_utils_strdup(strval);

	return IOTCON_ERR_NONE;
}

API iotcon_list_h iotcon_value_get_list(iotcon_value_h value)
{
	ic_val_list_s *real = (ic_val_list_s*)value;

	RETV_IF(NULL == value, NULL);
	RETVM_IF(IOTCON_TYPE_LIST != real->type, NULL, "IOTCON_ERR_PARAM(%d)", real->type);

	return real->list;
}

int ic_value_set_list(iotcon_value_h value, iotcon_list_h list)
{
	ic_val_list_s *real = (ic_val_list_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_LIST != real->type, IOTCON_ERR_PARAM, "IOTCON_ERR_PARAM(%d)",
			real->type);

	real->list = list;

	return IOTCON_ERR_NONE;
}

API iotcon_repr_h iotcon_value_get_repr(iotcon_value_h value)
{
	ic_val_repr_s *real = (ic_val_repr_s*)value;

	RETV_IF(NULL == value, NULL);
	RETVM_IF(IOTCON_TYPE_REPR != real->type, NULL, "IOTCON_ERR_PARAM(%d)", real->type);

	return real->repr;
}

int ic_value_set_repr(iotcon_value_h value, iotcon_repr_h repr)
{
	ic_val_repr_s *real = (ic_val_repr_s*)value;

	RETV_IF(NULL == value, IOTCON_ERR_PARAM);
	RETVM_IF(IOTCON_TYPE_REPR != real->type, IOTCON_ERR_PARAM, "IOTCON_ERR_PARAM(%d)",
			real->type);

	real->repr = repr;

	return IOTCON_ERR_NONE;
}

JsonNode* ic_repr_generate_json_value(iotcon_value_h value)
{
	RETV_IF(NULL == value, NULL);

	JsonNode *node = json_node_new(JSON_NODE_VALUE);
	int type = iotcon_value_get_type(value);

	if (IOTCON_TYPE_INT == type)
		json_node_set_int(node, iotcon_value_get_int(value));
	else if (IOTCON_TYPE_BOOL == type)
		json_node_set_boolean(node, iotcon_value_get_bool(value));
	else if (IOTCON_TYPE_DOUBLE == type)
		json_node_set_double(node, iotcon_value_get_double(value));
	else if (IOTCON_TYPE_STR == type)
		json_node_set_string(node, iotcon_value_get_str(value));
	else if (IOTCON_TYPE_NULL == type)
		node = json_node_init_null(node);
	else
		ERR("Invalid type(%d)", type);

	return node;
}

API iotcon_value_h ic_repr_parse_json_value(JsonNode *node)
{
	iotcon_value_h value = NULL;
	GType gtype = 0;

	RETV_IF(NULL == node, NULL);

	if (JSON_NODE_HOLDS_NULL(node)) {
		value = ic_value_new(IOTCON_TYPE_NULL);
		DBG("Set null value to node");
		return value;
	}

	gtype = json_node_get_value_type(node);
	if (G_TYPE_INT64 == gtype) {
		gint64 ival64 = json_node_get_int(node);
		if (INT_MAX < ival64 || ival64 < INT_MIN) {
			ERR("integer SHOULD NOT exceeds INT_MAX or INT_MIN. ival64(%lld)", ival64);
			return NULL;
		}
		value = ic_value_new(IOTCON_TYPE_INT);
		ic_value_set_int(value, ival64);
		DBG("Set int value(%d) to node", (int)ival64);
	}
	else if (G_TYPE_BOOLEAN == gtype) {
		bool bval = json_node_get_boolean(node);
		value = ic_value_new(IOTCON_TYPE_BOOL);
		ic_value_set_bool(value, bval);
		DBG("Set bool value(%d) to node", bval);
	}
	else if (G_TYPE_DOUBLE == gtype) {
		double dbval = json_node_get_double(node);
		value = ic_value_new(IOTCON_TYPE_DOUBLE);
		ic_value_set_double(value, dbval);
		DBG("Set double value(%lf) to node", dbval);
	}
	else if (G_TYPE_STRING == gtype) {
		const char *strval = json_node_get_string(node);
		value = ic_value_new(IOTCON_TYPE_STR);
		ic_value_set_str(value, strval);
		DBG("Set str value(%s) to node", strval);
	}
	else {
		ERR("gtype(%d) Fail", gtype);
	}

	return value;
}

void ic_repr_free_value(gpointer data)
{
	FN_CALL;
	iotcon_value_h value = data;
	int type = iotcon_value_get_type(value);

	switch (type) {
	case IOTCON_TYPE_INT:
	case IOTCON_TYPE_BOOL:
	case IOTCON_TYPE_DOUBLE:
	case IOTCON_TYPE_STR:
	case IOTCON_TYPE_NULL:
		DBG("value is basic. type(%d)", type);
		ic_repr_free_basic_value(value);
		break;
	case IOTCON_TYPE_LIST:
		DBG("value is list");
		iotcon_list_h iotlist_child = iotcon_value_get_list(value);
		iotcon_repr_free_list(iotlist_child);
		break;
	case IOTCON_TYPE_REPR:
		DBG("value is object");
		iotcon_repr_h repr_child = iotcon_value_get_repr(value);
		iotcon_repr_free(repr_child);
		break;
	default:
		ERR("Invalid type(%d)", type);
		break;
	}
}

void ic_repr_free_basic_value(iotcon_value_h iotvalue)
{
	FN_CALL;
	RET_IF(NULL == iotvalue);

	int type = iotcon_value_get_type(iotvalue);
	DBG("type(%d)", type);

	if (IOTCON_TYPE_STR == type) {
		char *str = iotcon_value_get_str(iotvalue);
		free(str);
	}
	free(iotvalue);
}
