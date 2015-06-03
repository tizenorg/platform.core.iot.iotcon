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
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <glib.h>
#include <json-glib/json-glib.h>

#include "iotcon-struct.h"
#include "iotcon-representation.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-ioty.h"
#include "ic-repr-list.h"
#include "ic-repr-value.h"
#include "ic-repr-obj.h"
#include "ic-repr.h"

void ic_repr_inc_ref_count(iotcon_repr_h val)
{
	RET_IF(NULL == val);
	RETM_IF(val->ref_count < 0, "Invalid Count(%d)", val->ref_count);

	val->ref_count++;
}

static bool _ic_repr_dec_ref_count(iotcon_repr_h val)
{
	bool ret;

	RETV_IF(NULL == val, -1);
	RETVM_IF(val->ref_count <= 0, false, "Invalid Count(%d)", val->ref_count);

	val->ref_count--;
	if (0 == val->ref_count)
		ret = true;
	else
		ret = false;

	return ret;
}

API iotcon_repr_h iotcon_repr_new()
{
	iotcon_repr_h ret_val;
	errno = 0;

	ret_val = calloc(1, sizeof(struct ic_repr_s));
	if (NULL == ret_val) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	ret_val->hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, free,
			ic_value_free);
	ic_repr_inc_ref_count(ret_val);

	return ret_val;
}

API int iotcon_repr_get_uri(iotcon_repr_h repr, const char **uri)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri, IOTCON_ERROR_INVALID_PARAMETER);

	*uri = repr->uri;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_set_uri(iotcon_repr_h repr, const char *uri)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	free(repr->uri);
	repr->uri = NULL;

	if (NULL == uri)
		return IOTCON_ERROR_NONE;

	repr->uri = strdup(uri);
	if (NULL == repr->uri) {
		ERR("strdup() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_get_resource_types(iotcon_repr_h repr, iotcon_str_list_s **types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = repr->res_types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_set_resource_types(iotcon_repr_h repr, iotcon_str_list_s *types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_str_list_free(repr->res_types);
	repr->res_types = NULL;

	if (NULL != types)
		repr->res_types = iotcon_str_list_clone(types);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_get_resource_interfaces(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, IOTCON_INTERFACE_NONE);

	return repr->interfaces;
}

API int iotcon_repr_set_resource_interfaces(iotcon_repr_h repr, int ifaces)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	repr->interfaces = ifaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_append_child(iotcon_repr_h parent, iotcon_repr_h child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	ic_repr_inc_ref_count(child);
	parent->children = g_list_append(parent->children, child);

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_foreach_children(iotcon_repr_h parent, iotcon_children_fn fn,
		void *user_data)
{
	GList *list, *next;

	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	list = parent->children;
	while (list) {
		next = list->next;
		if (IOTCON_FUNC_STOP == fn(list->data, user_data))
			break;
		list = next;
	}

	return IOTCON_ERROR_NONE;
}

API unsigned int iotcon_repr_get_children_count(iotcon_repr_h parent)
{
	RETV_IF(NULL == parent, 0);
	RETV_IF(NULL == parent->children, 0);

	return g_list_length(parent->children);
}

API int iotcon_repr_get_nth_child(iotcon_repr_h parent, int pos, iotcon_repr_h *child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == parent->children, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	*child = g_list_nth_data(parent->children, pos);
	if (NULL == *child) {
		ERR("g_list_nth_data() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	return IOTCON_ERROR_NONE;
}

API iotcon_str_list_s* iotcon_repr_get_key_list(iotcon_repr_h repr)
{
	GHashTableIter iter;
	gpointer key, value;
	iotcon_str_list_s *key_list = NULL;

	RETV_IF(NULL == repr, NULL);
	RETV_IF(NULL == repr->hash_table, NULL);

	g_hash_table_iter_init(&iter, repr->hash_table);
	while (g_hash_table_iter_next(&iter, &key, &value))
		key_list = iotcon_str_list_append(key_list, key);

	return key_list;
}

API int iotcon_repr_get_keys_count(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == repr->hash_table, IOTCON_ERROR_INVALID_PARAMETER);

	return g_hash_table_size(repr->hash_table);
}

static int _ic_repr_get_res_type_fn(const char *res_type, void *user_data)
{
	JsonArray *rt_array = user_data;
	json_array_add_string_element(rt_array, res_type);

	return IOTCON_FUNC_CONTINUE;
}

/*
 * A general result : {"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}
 */
static JsonObject* _ic_repr_data_generate_json(iotcon_repr_h cur_repr,
		unsigned int child_index)
{
	int i, ret, ifaces;
	char *iface_str;
	const char *uri;
	JsonObject *repr_obj = NULL;
	unsigned int rt_count = 0;
	JsonObject *prop_obj = NULL;
	iotcon_str_list_s *resource_types = NULL;

	RETV_IF(NULL == cur_repr, NULL);

	if (0 < iotcon_repr_get_keys_count(cur_repr)) {
		repr_obj = ic_obj_to_json(cur_repr);
		if (NULL == repr_obj) {
			ERR("ic_obj_to_json() Fail");
			json_object_unref(repr_obj);
			return NULL;
		}
	}
	else {
		repr_obj = json_object_new();
	}

	if (cur_repr->uri) {
		iotcon_repr_get_uri(cur_repr, &uri);
		json_object_set_string_member(repr_obj, IOTCON_KEY_URI, uri);
	}

	if (cur_repr->res_types)
		rt_count = iotcon_str_list_length(cur_repr->res_types);

	if (0 < rt_count || IOTCON_INTERFACE_NONE != cur_repr->interfaces) {
		prop_obj = json_object_new();
		json_object_set_object_member(repr_obj, IOTCON_KEY_PROPERTY, prop_obj);
	}

	if (0 < rt_count) {
		JsonArray *rt_array = json_array_new();

		ret = iotcon_repr_get_resource_types(cur_repr, &resource_types);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_repr_get_resource_types() Fail(%d)", ret);
			json_object_unref(repr_obj);
			return NULL;
		}

		ret = iotcon_str_list_foreach(resource_types, _ic_repr_get_res_type_fn,
				rt_array);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_str_list_foreach() Fail");
			json_object_unref(repr_obj);
			return NULL;
		}
		json_object_set_array_member(prop_obj, IOTCON_KEY_RESOURCETYPES, rt_array);
	}

	if (IOTCON_INTERFACE_NONE != cur_repr->interfaces) {
		JsonArray *if_array = json_array_new();
		ifaces = iotcon_repr_get_resource_interfaces(cur_repr);
		for (i = 1; i < IOTCON_INTERFACE_MAX; i = i << 1) {
			if (IOTCON_INTERFACE_NONE == (ifaces & i)) /* this interface not exist */
				continue;
			ret = ic_ioty_convert_interface_flag((ifaces & i), &iface_str);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("ic_ioty_convert_interface_flag(%d) Fail(%d)", i, ret);
				json_object_unref(repr_obj);
				json_array_unref(if_array);
				return NULL;
			}
			json_array_add_string_element(if_array, iface_str);
		}

		json_object_set_array_member(prop_obj, IOTCON_KEY_INTERFACES, if_array);
	}

	FN_END;
	return repr_obj;
}

static JsonObject* _ic_repr_data_generate_parent(iotcon_repr_h cur_repr,
		unsigned int child_index)
{
	FN_CALL;
	JsonObject *obj = _ic_repr_data_generate_json(cur_repr, child_index);
	if (NULL == obj) {
		ERR("_ic_repr_data_generate_json() Fail");
		return NULL;
	}

	return obj;
}

static JsonObject* _ic_repr_data_generate_child(iotcon_repr_h cur_repr,
		unsigned int child_index)
{
	FN_CALL;
	JsonObject *obj = _ic_repr_data_generate_json(cur_repr, child_index);
	if (NULL == obj) {
		ERR("_ic_repr_data_generate_json() Fail");
		return NULL;
	}

	return obj;
}

/*
 * A general result : {oc:[{"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}},
 * 						{"href":"/a/child","rep":{"string":"World","double_val":5.7},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}]}
 */
static JsonObject* _ic_repr_generate_json(iotcon_repr_h repr)
{
	JsonObject *repr_obj = NULL;
	JsonObject *root_obj = NULL;
	JsonArray *root_array = NULL;
	unsigned int child_count = 0;
	unsigned int child_index = 0;
	iotcon_repr_h child_repr = NULL;

	RETV_IF(NULL == repr, NULL);

	root_obj = json_object_new();
	root_array = json_array_new();

	if (repr->children) {
		child_count = iotcon_repr_get_children_count(repr);
	}

	repr_obj = _ic_repr_data_generate_parent(repr, child_index);
	if (NULL == repr_obj) {
		ERR("_ic_repr_data_generate_parent() Fali(NULL == repr_obj)");
		json_object_unref(root_obj);
		json_array_unref(root_array);
		return NULL;
	}
	json_array_add_object_element(root_array, repr_obj);

	for (child_index = 0; child_index < child_count; child_index++) {
		iotcon_repr_get_nth_child(repr, child_index, &child_repr);
		repr_obj = _ic_repr_data_generate_child(child_repr, child_index);
		if (NULL == repr_obj) {
			ERR("_ic_repr_data_generate_child() Fali(NULL == repr_obj)");
			json_object_unref(root_obj);
			json_array_unref(root_array);
			return NULL;
		}
		json_array_add_object_element(root_array, repr_obj);
	}

	json_object_set_array_member(root_obj, IOTCON_KEY_OC, root_array);

	return root_obj;
}

char* ic_repr_generate_json(iotcon_repr_h repr, bool set_pretty)
{
	JsonNode *root_node = NULL;
	char *json_data = NULL;

	JsonObject *obj = _ic_repr_generate_json(repr);
	if (NULL == obj) {
		ERR("ic_repr_generate_json() Fail");
		return NULL;
	}

	JsonGenerator *gen = json_generator_new();
#if JSON_CHECK_VERSION(0,14,0)
	json_generator_set_pretty(gen, set_pretty);
#endif

	root_node = json_node_new(JSON_NODE_OBJECT);
	json_node_set_object(root_node, obj);
	json_generator_set_root(gen, root_node);

	json_data = json_generator_to_data(gen, NULL);
	DBG("result : %s", json_data);

	return json_data;
}

/*
 * A general input : {"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}
 */
iotcon_repr_h ic_repr_parse_json(const char *json_string)
{
	const char *iface_str = NULL;
	const char *rtype_str = NULL;
	const char *uri_value = NULL;
	iotcon_interface_e iface_flag;

	RETV_IF(NULL == json_string, NULL);

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
		repr = ic_obj_from_json(root_obj);
		if (NULL == repr) {
			ERR("ic_obj_from_json() Fail()");
			g_object_unref(parser);
			return NULL;
		}
	}
	else {
		repr = iotcon_repr_new();
	}

	if (json_object_has_member(root_obj, IOTCON_KEY_URI)) {
		uri_value = json_object_get_string_member(root_obj, IOTCON_KEY_URI);
		iotcon_repr_set_uri(repr, uri_value);
	}

	if (json_object_has_member(root_obj, IOTCON_KEY_PROPERTY)) {
		JsonObject *property_obj = json_object_get_object_member(root_obj,
		IOTCON_KEY_PROPERTY);

		if (json_object_has_member(property_obj, IOTCON_KEY_RESOURCETYPES)) {
			iotcon_str_list_s *res_types = NULL;
			JsonArray *rt_array = json_object_get_array_member(property_obj,
			IOTCON_KEY_RESOURCETYPES);
			unsigned int rt_index = 0;
			unsigned int rt_count = json_array_get_length(rt_array);

			for (rt_index = 0; rt_index < rt_count; rt_index++) {
				rtype_str = json_array_get_string_element(rt_array, rt_index);
				res_types = iotcon_str_list_append(res_types, rtype_str);
			}
			iotcon_repr_set_resource_types(repr, res_types);
		}
		if (json_object_has_member(property_obj, IOTCON_KEY_INTERFACES)) {
			JsonArray *if_array = json_object_get_array_member(property_obj,
			IOTCON_KEY_INTERFACES);
			unsigned int if_count = json_array_get_length(if_array);
			unsigned int if_index;
			int ifaces = IOTCON_INTERFACE_NONE;

			for (if_index = 0; if_index < if_count; if_index++) {
				iface_str = json_array_get_string_element(if_array, if_index);
				ret = ic_ioty_convert_interface_string(iface_str, &iface_flag);
				ifaces |= iface_flag;
			}
			iotcon_repr_set_resource_interfaces(repr, ifaces);

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

	if (false == _ic_repr_dec_ref_count(repr))
		return;

	free(repr->uri);

	/* (GDestroyNotify) : iotcon_repr_h is proper type than gpointer */
	g_list_free_full(repr->children, (GDestroyNotify)iotcon_repr_free);

	/* repr->res_types COULD be not null */
	if (repr->res_types)
		iotcon_str_list_free(repr->res_types);
	g_hash_table_destroy(repr->hash_table);
	free(repr);

	FN_END;
}

static void _ic_repr_obj_clone(char *key, iotcon_value_h src_val, iotcon_repr_h dest_repr)
{
	FN_CALL;
	int type, ret;
	iotcon_value_h value, copied_val;
	iotcon_list_h child_list, copied_list;
	iotcon_repr_h child_repr, copied_repr;

	type = src_val->type;
	switch (type) {
	case IOTCON_TYPE_INT:
	case IOTCON_TYPE_BOOL:
	case IOTCON_TYPE_DOUBLE:
	case IOTCON_TYPE_STR:
	case IOTCON_TYPE_NULL:
		copied_val = ic_value_clone(src_val);
		if (NULL == copied_val) {
			ERR("ic_value_clone() Fail");
			return;
		}
		ic_obj_set_value(dest_repr, ic_utils_strdup(key), copied_val);
		break;
	case IOTCON_TYPE_LIST:
		ret = ic_value_get_list(src_val, &child_list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_value_get_list() Fail(%d)", ret);
			return;
		}

		copied_list = ic_list_clone(child_list);
		if (NULL == copied_list) {
			ERR("ic_list_clone() Fail");
			return;
		}

		value = ic_value_new_list(copied_list);
		if (NULL == value) {
			ERR("ic_value_new_list() Fail");
			iotcon_list_free(copied_list);
			return;
		}

		ic_obj_set_value(dest_repr, key, value);
		break;
	case IOTCON_TYPE_REPR:
		ret = ic_value_get_repr(src_val, &child_repr);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_value_get_repr() Fail(%d)", ret);
			return;
		}

		copied_repr = iotcon_repr_clone(child_repr);
		if (NULL == copied_repr) {
			ERR("ic_list_clone() Fail");
			return;
		}

		value = ic_value_new_repr(copied_repr);
		if (NULL == value) {
			ERR("ic_value_new_repr(%p) Fail", copied_repr);
			return;
		}

		ic_obj_set_value(dest_repr, key, value);
		break;
	default:
		ERR("Invalid type(%d)", type);
		return;
	}
}

API iotcon_repr_h iotcon_repr_clone(const iotcon_repr_h src)
{
	FN_CALL;
	GList *node;
	iotcon_repr_h dest, copied_repr;
	iotcon_str_list_s *list = NULL;

	RETV_IF(NULL == src, NULL);

	dest = iotcon_repr_new();
	if (NULL == dest) {
		ERR("iotcon_repr_new() Fail");
		return NULL;
	}

	if (src->uri) {
		dest->uri = strdup(src->uri);
		if (NULL == dest->uri) {
			ERR("strdup() Fail");
			iotcon_repr_free(dest);
			return NULL;
		}
	}

	if (src->interfaces)
		dest->interfaces = src->interfaces;

	if (src->res_types) {
		list = iotcon_str_list_clone(src->res_types);
		if (NULL == list) {
			ERR("iotcon_str_list_clone() Fail");
			iotcon_repr_free(dest);
			return NULL;
		}
		dest->res_types = list;
	}

	for (node = g_list_first(src->children); node; node = node->next) {
		copied_repr = iotcon_repr_clone((iotcon_repr_h)node->data);
		if (NULL == copied_repr) {
			ERR("iotcon_repr_clone(child) Fail");
			iotcon_repr_free(dest);
			return NULL;
		}
		dest->children = g_list_append(dest->children, copied_repr);
	}

	g_hash_table_foreach(src->hash_table, (GHFunc)_ic_repr_obj_clone, dest);

	return dest;
}

API char* iotcon_repr_generate_json(iotcon_repr_h repr)
{
	return ic_repr_generate_json(repr, TRUE);
}
