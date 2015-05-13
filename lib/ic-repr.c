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
#include <limits.h>

#include <json-glib/json-glib.h>

#include "iotcon.h"

#include "ic-common.h"
#include "ic-struct.h"
#include "ic-utils.h"
#include "ic-repr-list.h"
#include "ic-repr-value.h"
#include "ic-repr.h"

API iotcon_repr_h iotcon_repr_new()
{
	iotcon_repr_h ret_val;
	errno = 0;

	ret_val = calloc(1, sizeof(struct ic_repr_s));
	if (NULL == ret_val) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	ret_val->hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
			ic_repr_free_value);

	return ret_val;
}

API const char* iotcon_repr_get_uri(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, NULL);

	return repr->uri;
}

API int iotcon_repr_set_uri(iotcon_repr_h repr, const char *uri)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == uri, IOTCON_ERR_PARAM);

	repr->uri = ic_utils_strdup(uri);
	if (NULL == repr->uri) {
		ERR("ic_utils_strdup() Fail");
		return IOTCON_ERR_MEMORY;
	}

	return IOTCON_ERR_NONE;
}

API void iotcon_repr_get_resource_types(iotcon_repr_h repr, iotcon_resourcetype_fn fn,
		void *user_data)
{
	RET_IF(NULL == repr);

	/* (GFunc) : fn needs to use "const" qualifier */
	return g_list_foreach(repr->res_types, (GFunc)fn, user_data);
}

API int iotcon_repr_get_resource_types_count(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);

	return g_list_length(repr->res_types);
}

API int iotcon_repr_append_resource_types(iotcon_repr_h repr, const char *type)
{
	char *res_type = NULL;

	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == type, IOTCON_ERR_PARAM);

	res_type = ic_utils_strdup(type);
	if (NULL == res_type) {
		ERR("ic_utils_strdup() Fail");
		return IOTCON_ERR_MEMORY;
	}

	repr->res_types = g_list_append(repr->res_types, res_type);

	return IOTCON_ERR_NONE;
}

API void iotcon_repr_get_resource_interfaces(iotcon_repr_h repr, iotcon_interface_fn fn,
		void *user_data)
{
	RET_IF(NULL == repr);

	/* (GFunc) : fn needs to use "const" qualifier */
	return g_list_foreach(repr->interfaces, (GFunc)fn, user_data);
}

API int iotcon_repr_get_resource_interfaces_count(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);

	return g_list_length(repr->interfaces);
}

API int iotcon_repr_append_resource_interfaces(iotcon_repr_h repr, const char *interface)
{
	char *res_interface = NULL;

	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == interface, IOTCON_ERR_PARAM);

	res_interface = ic_utils_strdup(interface);
	if (NULL == res_interface) {
		ERR("ic_utils_strdup() Fail");
		return IOTCON_ERR_MEMORY;
	}

	repr->interfaces = g_list_append(repr->interfaces, res_interface);
	return IOTCON_ERR_NONE;
}

API int iotcon_repr_get_int(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	iotcon_value_h value = g_hash_table_lookup(repr->hash_table, key);
	if(NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERR_NO_DATA;
	}

	ic_basic_s *real = (ic_basic_s*)value;
	RETV_IF(IOTCON_TYPE_INT != real->type, IOTCON_ERR_PARAM);

	return real->val.i;
}

API int iotcon_repr_set_int(iotcon_repr_h repr, const char *key, int ival)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_INT);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_INT) Fail");
		return IOTCON_ERR_MEMORY;
	}

	ic_basic_s *real = (ic_basic_s*)value;

	real->val.i = ival;

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

API bool iotcon_repr_get_bool(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, false);
	RETV_IF(NULL == key, false);

	iotcon_value_h value = g_hash_table_lookup(repr->hash_table, key);
	if(NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return false;
	}

	ic_basic_s *real = (ic_basic_s*)value;
	RETV_IF(IOTCON_TYPE_BOOL != real->type, false);

	return real->val.b;
}

API int iotcon_repr_set_bool(iotcon_repr_h repr, const char *key, bool bval)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_BOOL);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_BOOL) Fail");
		return IOTCON_ERR_MEMORY;
	}

	ic_basic_s *real = (ic_basic_s*)value;

	real->val.b = bval;

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

API double iotcon_repr_get_double(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, 0);
	RETV_IF(NULL == key, 0);

	iotcon_value_h value = g_hash_table_lookup(repr->hash_table, key);
	if(NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return 0;
	}

	ic_basic_s *real = (ic_basic_s*)value;
	RETV_IF(IOTCON_TYPE_DOUBLE != real->type, 0);

	return real->val.d;
}

API int iotcon_repr_set_double(iotcon_repr_h repr, const char *key, double dbval)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_DOUBLE);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_DOUBLE) Fail");
		return IOTCON_ERR_MEMORY;
	}

	ic_basic_s *real = (ic_basic_s*)value;

	real->val.d = dbval;

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

API char* iotcon_repr_get_str(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, NULL);
	RETV_IF(NULL == key, NULL);

	iotcon_value_h value = g_hash_table_lookup(repr->hash_table, key);
	if(NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return NULL;
	}

	ic_basic_s *real = (ic_basic_s*)value;
	RETV_IF(IOTCON_TYPE_STR != real->type, NULL);

	return real->val.s;
}

API int iotcon_repr_set_str(iotcon_repr_h repr, const char *key, const char *strval)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_STR);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_STR) Fail");
		return IOTCON_ERR_MEMORY;
	}

	ic_basic_s *real = (ic_basic_s*)value;

	real->val.s = ic_utils_strdup(strval);

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

API bool iotcon_repr_is_null(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, false);
	RETV_IF(NULL == key, false);

	iotcon_value_h value = (iotcon_value_h) g_hash_table_lookup(repr->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return false;
	}

	ic_basic_s *real = (ic_basic_s*)value;

	return (IOTCON_TYPE_NULL == real->type) ? true : false;
}

API int iotcon_repr_set_null(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_NULL);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_NULL) Fail");
		return IOTCON_ERR_MEMORY;
	}

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

API iotcon_list_h iotcon_repr_get_list(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, NULL);
	RETV_IF(NULL == key, NULL);

	iotcon_value_h value = g_hash_table_lookup(repr->hash_table, key);
	if(NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return NULL;
	}

	ic_val_list_s *real = (ic_val_list_s*)value;
	RETV_IF(IOTCON_TYPE_LIST != real->type, NULL);

	return real->list;
}

API int iotcon_repr_replace_list(iotcon_repr_h repr, const char *key, iotcon_list_h list)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_LIST);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_LIST) Fail");
		return IOTCON_ERR_MEMORY;
	}

	ic_val_list_s *real = (ic_val_list_s*)value;
	RETV_IF(IOTCON_TYPE_LIST != real->type, IOTCON_ERR_PARAM);

	real->list = list;

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

API iotcon_repr_h iotcon_repr_get_repr(iotcon_repr_h repr,
		const char *key)
{
	RETV_IF(NULL == repr, NULL);
	RETV_IF(NULL == key, NULL);

	iotcon_value_h value = g_hash_table_lookup(repr->hash_table, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return NULL;
	}

	ic_val_repr_s *real = (ic_val_repr_s*)value;
	RETV_IF(IOTCON_TYPE_REPR != real->type, NULL);

	return real->repr;
}

API int iotcon_repr_replace_repr(iotcon_repr_h dest, const char *key, iotcon_repr_h src)
{
	RETV_IF(NULL == dest, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);
	RETV_IF(NULL == src, IOTCON_ERR_PARAM);

	iotcon_value_h value = ic_value_new(IOTCON_TYPE_REPR);
	if (NULL == value) {
		ERR("ic_value_new(IOTCON_TYPE_REPR) Fail");
		return IOTCON_ERR_MEMORY;
	}

	ic_val_repr_s *real = (ic_val_repr_s*)value;

	real->repr = src;

	g_hash_table_replace(dest->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

int ic_repr_get_value(iotcon_repr_h repr, const char *key, iotcon_value_h *retval)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);

	*retval = g_hash_table_lookup(repr->hash_table, key);
	if (NULL == retval)
		return IOTCON_ERR_NO_DATA;
	else
		return IOTCON_ERR_NONE;
}

int ic_repr_replace_value(iotcon_repr_h repr, const char *key, iotcon_value_h value)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERR_PARAM);
	RETV_IF(NULL == value, IOTCON_ERR_PARAM);

	g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);

	return IOTCON_ERR_NONE;
}

API int iotcon_repr_append_child(iotcon_repr_h parent, iotcon_repr_h child)
{
	RETV_IF(NULL == parent, IOTCON_ERR_PARAM);
	RETV_IF(NULL == child, IOTCON_ERR_PARAM);

	parent->children = g_list_append(parent->children, child);

	return IOTCON_ERR_NONE;
}

API GList* iotcon_repr_get_children(iotcon_repr_h parent)
{
    RETV_IF(NULL == parent, NULL);

	return parent->children;
}

API int iotcon_repr_get_children_count(iotcon_repr_h parent)
{
	RETV_IF(NULL == parent, IOTCON_ERR_PARAM);
	RETV_IF(NULL == parent->children, IOTCON_ERR_PARAM);

	return g_list_length(parent->children);
}

API iotcon_repr_h iotcon_repr_get_nth_child(iotcon_repr_h parent, int index)
{
	RETV_IF(NULL == parent, NULL);
	RETV_IF(NULL == parent->children, NULL);

	return g_list_nth_data(parent->children, index);
}

API GList* iotcon_repr_get_key_list(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, NULL);
	RETV_IF(NULL == repr->hash_table, NULL);

	return g_hash_table_get_keys(repr->hash_table);
}

API int iotcon_repr_get_keys_count(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == repr->hash_table, IOTCON_ERR_PARAM);

	return g_hash_table_size(repr->hash_table);
}

bool ic_repr_has_value(iotcon_repr_h repr, const char *key)
{
	RETV_IF(NULL == repr, false);
	RETV_IF(NULL == key, false);

	return g_hash_table_contains(repr->hash_table, key);
}

JsonObject* ic_repr_generate_json_repr(iotcon_repr_h repr, int *err_code)
{
	char *key;
	unsigned int i = 0;
	GList *key_list = NULL;
	JsonObject *json_obj = NULL;
	int error_code = IOTCON_ERR_NONE;

	JsonNode *node_child = NULL;
	JsonArray *array_child = NULL;
	iotcon_list_h iotlist_child = NULL;
	iotcon_repr_h repr_child = NULL;

	RETV_IF(NULL == repr, NULL);

	json_obj = json_object_new();
	key_list = iotcon_repr_get_key_list(repr);
	ic_utils_print_str_list(key_list);

	for (i = 0; i < g_list_length(key_list); i++) {
		int type = 0;
		iotcon_value_h iotvalue_child = NULL;

		key = g_list_nth_data(key_list, i);
		ic_repr_get_value(repr, key, &iotvalue_child);
		type = iotcon_value_get_type(iotvalue_child);
		switch (type) {
		case IOTCON_TYPE_INT:
		case IOTCON_TYPE_BOOL:
		case IOTCON_TYPE_DOUBLE:
		case IOTCON_TYPE_STR:
		case IOTCON_TYPE_NULL:
			node_child = ic_repr_generate_json_value(iotvalue_child);
			if (NULL == node_child) {
				ERR("ic_repr_generate_json_value() Fail(NULL == node_child)");
				json_object_unref(json_obj);
				g_list_free(key_list);
				*err_code = IOTCON_ERR_PARAM;
				return NULL;
			}
			json_object_set_member(json_obj, key, node_child);
			break;
		case IOTCON_TYPE_LIST:
			iotlist_child = iotcon_value_get_list(iotvalue_child);
			array_child = ic_repr_generate_json_array(iotlist_child);
			if (NULL == array_child) {
				ERR("ic_repr_generate_json_array() Fail(NULL == array_child)");
				json_object_unref(json_obj);
				g_list_free(key_list);
				*err_code = IOTCON_ERR_PARAM;
				return NULL;
			}
			node_child = json_node_new(JSON_NODE_ARRAY);
			json_node_set_array(node_child, array_child);
			json_object_set_member(json_obj, key, node_child);
			break;
		case IOTCON_TYPE_REPR:
			repr_child = iotcon_value_get_repr(iotvalue_child);
			JsonObject *obj = ic_repr_generate_json_repr(repr_child, &error_code);
			if (NULL == obj) {
				ERR("ic_repr_generate_json_repr() Fail(NULL == obj)");
				json_object_unref(json_obj);
				g_list_free(key_list);
				*err_code = IOTCON_ERR_PARAM;
				return NULL;
			}
			node_child = json_node_new(JSON_NODE_OBJECT);
			json_node_set_object(node_child, obj);
			json_object_set_member(json_obj, key, node_child);
			break;
		default:
			ERR("Invalid type(%d)", type);
			json_object_unref(json_obj);
			g_list_free(key_list);
			*err_code = IOTCON_ERR_PARAM;
			return NULL;
		}
	}
	g_list_free(key_list);

	JsonObject *ret_obj = json_object_new();
	json_object_set_object_member(ret_obj, IOTCON_KEY_REP, json_obj);

	return ret_obj;
}

void _ic_repr_get_res_type_fn(const char *res_type, void *user_data)
{
	JsonArray *rt_array = user_data;
	json_array_add_string_element(rt_array, res_type);
	DBG("res_type(%s)", res_type);
}

void _ic_repr_get_res_interface_fn(const char *res_if, void *user_data)
{
	JsonArray *if_array = user_data;
	json_array_add_string_element(if_array, res_if);
	DBG("res_if(%s)", res_if);
}

JsonObject* _ic_repr_generate_json_foreach(iotcon_repr_h cur_repr,
		unsigned int child_index)
{
	FN_CALL;
	JsonObject *repr_obj = NULL;
	unsigned int rt_count = 0;
	unsigned int if_count = 0;
	JsonObject *prop_obj = NULL;

	if (0 < iotcon_repr_get_keys_count(cur_repr)) {
		int error_code = IOTCON_ERR_NONE;
		DBG("Representation has \"%s\" key", IOTCON_KEY_REP);

		repr_obj = ic_repr_generate_json_repr(cur_repr, &error_code);
		if (IOTCON_ERR_NONE != error_code) {
			ERR("ic_repr_generate_json_repr() Fail(%d)", error_code);
			return NULL;
		}
	}

	if (NULL == repr_obj) // cur_repr didn't have "rep" key and repr_obj was not made.
		repr_obj = json_object_new();

	if (cur_repr->uri) {
		const char *uri = iotcon_repr_get_uri(cur_repr);
		DBG("Representation has \"%s\" key", IOTCON_KEY_URI);
		json_object_set_string_member(repr_obj, IOTCON_KEY_URI, uri);
	}

	if (cur_repr->res_types) {
		DBG("Representation has \"%s\" key", IOTCON_KEY_RESOURCETYPES);
		rt_count = iotcon_repr_get_resource_types_count(cur_repr);
	}
	if (cur_repr->interfaces) {
		DBG("Representation has \"%s\" key", IOTCON_KEY_INTERFACES);
		if_count = iotcon_repr_get_resource_interfaces_count(cur_repr);
	}

	if (0 < rt_count || 0 < if_count) {
		prop_obj = json_object_new();
		json_object_set_object_member(repr_obj, IOTCON_KEY_PROPERTY, prop_obj);
	}

	if (0 < rt_count) {
		DBG("Representation has \"%s\" key. count(%d)", IOTCON_KEY_RESOURCETYPES,
				rt_count);
		JsonArray *rt_array = json_array_new();
		iotcon_repr_get_resource_interfaces(cur_repr, _ic_repr_get_res_type_fn, rt_array);
		json_object_set_array_member(prop_obj, IOTCON_KEY_RESOURCETYPES, rt_array);
	}

	if (0 < if_count) {
		DBG("Representation has \"%s\" key. count(%d)", IOTCON_KEY_INTERFACES, if_count);
		JsonArray *if_array = json_array_new();
		iotcon_repr_get_resource_types(cur_repr, _ic_repr_get_res_interface_fn, if_array);
		json_object_set_array_member(prop_obj, IOTCON_KEY_INTERFACES, if_array);
	}

	return repr_obj;
}

char* ic_repr_generate_json(iotcon_repr_h repr)
{
	JsonObject *repr_obj = NULL;
	gchar *json_data = NULL;
	JsonObject *root_obj = NULL;
	JsonNode *root_node = NULL;
	JsonArray *root_array = NULL;
	GList *children = NULL;
	unsigned int child_count = 0;
	unsigned int child_index = 0;
	iotcon_repr_h cur_repr = NULL;

	RETV_IF(NULL == repr, NULL);

	root_obj = json_object_new();
	root_array = json_array_new();

	if (repr->children) {
		children = iotcon_repr_get_children(repr);
		child_count = g_list_length(children);
	}

	cur_repr = repr;
	for (child_index = 0; child_index < child_count + 1; child_index++) {
		if (1 <= child_index) { // child representation
			cur_repr = iotcon_repr_get_nth_child(repr, child_index - 1);
			DBG("[Child representation]", child_index);
		}
		else {
			DBG("[Parent representation]");
		}
		repr_obj = _ic_repr_generate_json_foreach(cur_repr, child_index);
		if (NULL == repr_obj) {
			ERR("_ic_repr_generate_json_foreach() Fali(NULL == repr_obj)");
			json_object_unref(root_obj);
			json_array_unref(root_array);
			return NULL;
		}
		json_array_add_object_element(root_array, repr_obj);
	}

	json_object_set_array_member(root_obj, IOTCON_KEY_OC, root_array);

	JsonGenerator *gen = json_generator_new();
	root_node = json_node_new(JSON_NODE_OBJECT);
	json_node_set_object(root_node, root_obj);
	json_generator_set_root(gen, root_node);

	json_data = json_generator_to_data(gen, NULL);
	DBG("result : %s", json_data);

	return json_data;
}

iotcon_repr_h ic_repr_parse_json_obj(JsonObject *obj, int *err_code)
{
	iotcon_repr_h repr = iotcon_repr_new();
	GList *key_list = json_object_get_members(obj);
	ic_utils_print_str_list(key_list);
	char *key;
	unsigned int i = 0;

	for (i = 0; i < g_list_length(key_list); i++) {
		key = g_list_nth_data(key_list, i);

		// search child object recursively
		JsonNode *child_node = json_object_get_member(obj, key);
		if (NULL == child_node) {
			ERR("json_object_get_member() Fail(NULL == child_node)");
			iotcon_repr_free(repr);
			*err_code = IOTCON_ERR_PARAM;
			return NULL;
		}
		if (JSON_NODE_HOLDS_NULL(child_node) || JSON_NODE_HOLDS_VALUE(child_node)) {
			iotcon_value_h value = ic_repr_parse_json_value(child_node);
			if (NULL == value) {
				ERR("ic_repr_parse_json_value() Fail(NULL == value)");
				iotcon_repr_free(repr);
				*err_code = IOTCON_ERR_PARAM;
				return NULL;
			}
			ic_repr_replace_value(repr, key, value);
		}
		else if (JSON_NODE_HOLDS_ARRAY(child_node)) {
			JsonArray *child_array = json_node_get_array(child_node);
			iotcon_list_h list = ic_repr_parse_json_array(child_array);
			if (NULL == list) {
				ERR("ic_repr_parse_json_array() Fail(NULL == list)");
				iotcon_repr_free(repr);
				*err_code = IOTCON_ERR_PARAM;
				return NULL;
			}
			iotcon_repr_replace_list(repr, key, list);
		}
		else if (JSON_NODE_HOLDS_OBJECT(child_node)) {
			int error_code = IOTCON_ERR_NONE;
			JsonObject *child_obj = json_node_get_object(child_node);
			JsonObject *repr_obj = json_object_get_object_member(child_obj,
			IOTCON_KEY_REP);
			iotcon_repr_h ret_repr = ic_repr_parse_json_obj(repr_obj, &error_code);
			if (NULL == ret_repr) {
				ERR("ic_repr_parse_json_obj() Fail(NULL == ret_repr)");
				iotcon_repr_free(repr);
				*err_code = IOTCON_ERR_PARAM;
				return NULL;
			}
			iotcon_repr_replace_repr(repr, key, ret_repr);
		}
		else {
			ERR("node type(%d) Fail", json_node_get_node_type(child_node));
			iotcon_repr_free(repr);
			*err_code = IOTCON_ERR_PARAM;
			return NULL;
		}
	}
	g_list_free(key_list);

	return repr;
}

iotcon_repr_h ic_repr_parse_json(const char *json_string)
{
	const char *uri_value = NULL;

	RETV_IF(NULL == json_string, NULL);

	// incoming json format example : {"href":"/a/address","rep":{"text":"Hello world"}}
	DBG("input str : %s", json_string);

	GError *error = NULL;
	gboolean ret = FALSE;
	JsonParser *parser = json_parser_new();
	ret = json_parser_load_from_data(parser, json_string, strlen(json_string), &error);
	if (FALSE == ret) {
		ERR("json_parser_load_from_data() Fail(%s)", error->message);
		g_error_free(error);
		g_object_unref(parser);
		return NULL;
	}

	JsonObject *root_obj = json_node_get_object(json_parser_get_root(parser));

	iotcon_repr_h repr = NULL;
	if (json_object_has_member(root_obj, IOTCON_KEY_REP)) {
		int err_code = IOTCON_ERR_NONE;

		DBG("Representation has \"%s\" key", IOTCON_KEY_REP);
		JsonObject *repr_obj = json_object_get_object_member(root_obj, IOTCON_KEY_REP);
		repr = ic_repr_parse_json_obj(repr_obj, &err_code);
		if (IOTCON_ERR_NONE != err_code) {
			ERR("ic_repr_parse_json_obj() Fail(%d)", err_code);
			g_object_unref(parser);
			return NULL;
		}
	}

	// root_obj didn't have "repr" key and iotcon_repr_h was not made.
	if (NULL == repr)
		repr = iotcon_repr_new();

	if (json_object_has_member(root_obj, IOTCON_KEY_URI)) {
		uri_value = json_object_get_string_member(root_obj, IOTCON_KEY_URI);
		DBG("Representation has \"%s\" key (%s)", IOTCON_KEY_URI, uri_value);
		iotcon_repr_set_uri(repr, uri_value);
	}

	if (json_object_has_member(root_obj, IOTCON_KEY_PROPERTY)) {
		DBG("Representation has \"%s\" key", IOTCON_KEY_PROPERTY);
		JsonObject *property_obj = json_object_get_object_member(root_obj,
		IOTCON_KEY_PROPERTY);

		if (json_object_has_member(property_obj, IOTCON_KEY_RESOURCETYPES)) {
			DBG("Representation has \"%s\" key", IOTCON_KEY_RESOURCETYPES);
			JsonArray *rt_array = json_object_get_array_member(property_obj,
			IOTCON_KEY_RESOURCETYPES);
			unsigned int rt_count = json_array_get_length(rt_array);

			DBG("rt_count:%d", rt_count);

			unsigned int rt_index = 0;
			for (rt_index = 0; rt_index < rt_count; rt_index++) {
				iotcon_repr_append_resource_types(repr,
						json_array_get_string_element(rt_array, rt_index));
			}
		}
		if (json_object_has_member(property_obj, IOTCON_KEY_INTERFACES)) {
			DBG("Representation has \"%s\" key", IOTCON_KEY_INTERFACES);
			JsonArray *if_array = json_object_get_array_member(property_obj,
			IOTCON_KEY_INTERFACES);
			unsigned int if_count = json_array_get_length(if_array);

			DBG("if_count:%d", if_count);

			unsigned int if_index = 0;
			for (if_index = 0; if_index < if_count; if_index++)
				iotcon_repr_append_resource_interfaces(repr,
						json_array_get_string_element(if_array, if_index));
		}
	}

	if (NULL == repr) {
		ERR("repr is NULL");
		g_object_unref(parser);
		return NULL;
	}

	g_object_unref(parser);

	FN_END;

	return repr;
}

API void iotcon_repr_free(iotcon_repr_h repr)
{
	FN_CALL;
	RET_IF(NULL == repr);

	free(repr->uri);

	/* (GDestroyNotify) : iotcon_repr_h is proper type than gpointer */
	g_list_free_full(repr->children, (GDestroyNotify)iotcon_repr_free);
	g_list_free_full(repr->interfaces, g_free);
	g_list_free_full(repr->res_types, g_free);
	g_hash_table_destroy(repr->hash_table);
	free(repr);

	FN_END;
}

API void iotcon_repr_print(iotcon_repr_h repr)
{
	FN_CALL;
	gchar *json_data = NULL;
	JsonNode *root_node = NULL;
	int error_code = IOTCON_ERR_NONE;

	JsonObject *repr_obj = ic_repr_generate_json_repr(repr, &error_code);
	if (NULL == repr_obj) {
		ERR("ic_repr_generate_json_repr() Fail(NULL == repr_obj)");
		return;
	}

	root_node = json_node_new(JSON_NODE_OBJECT);
	JsonGenerator *gen = json_generator_new();
	json_generator_set_pretty(gen, TRUE);
	json_node_set_object(root_node, repr_obj);
	json_generator_set_root(gen, root_node);

	json_data = json_generator_to_data(gen, NULL);
	DBG("\n%s", json_data);
}
