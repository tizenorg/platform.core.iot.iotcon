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
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource.h"
#include "icl-resource-types.h"
#include "icl-repr-list.h"
#include "icl-repr-value.h"
#include "icl-repr-obj.h"
#include "icl-repr.h"

void icl_repr_inc_ref_count(iotcon_repr_h val)
{
	RET_IF(NULL == val);
	RETM_IF(val->ref_count < 0, "Invalid Count(%d)", val->ref_count);

	val->ref_count++;
}

static bool _icl_repr_dec_ref_count(iotcon_repr_h val)
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

	ret_val = calloc(1, sizeof(struct icl_repr_s));
	if (NULL == ret_val) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	ret_val->hash_table = g_hash_table_new_full(g_str_hash, g_str_equal, free,
			icl_value_free);
	icl_repr_inc_ref_count(ret_val);

	return ret_val;
}

API int iotcon_repr_get_uri_path(iotcon_repr_h repr, const char **uri_path)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = repr->uri_path;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_set_uri_path(iotcon_repr_h repr, const char *uri_path)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	free(repr->uri_path);
	repr->uri_path = NULL;

	if (NULL == uri_path)
		return IOTCON_ERROR_INVALID_PARAMETER;

	repr->uri_path = strdup(uri_path);
	if (NULL == repr->uri_path) {
		ERR("strdup() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_get_resource_types(iotcon_repr_h repr, iotcon_resource_types_h *types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = repr->res_types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_set_resource_types(iotcon_repr_h repr, iotcon_resource_types_h types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_resource_types_free(repr->res_types);
	repr->res_types = NULL;

	if (types)
		repr->res_types = icl_resource_types_ref(types);

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

	RETV_IF(ifaces <= IOTCON_INTERFACE_NONE || ICL_INTERFACE_MAX < ifaces,
			IOTCON_ERROR_INVALID_PARAMETER);

	repr->interfaces = ifaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_repr_append_child(iotcon_repr_h parent, iotcon_repr_h child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	icl_repr_inc_ref_count(child);
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

API int iotcon_repr_foreach(iotcon_repr_h repr, iotcon_repr_fn fn, void *user_data)
{
	GHashTableIter iter;
	gpointer key;

	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == fn, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_iter_init(&iter, repr->hash_table);
	while (g_hash_table_iter_next(&iter, &key, NULL)) {
		if (IOTCON_FUNC_STOP == fn(repr, key, user_data))
			break;
	}

	return IOTCON_ERROR_NONE;
}

API unsigned int iotcon_repr_get_keys_count(iotcon_repr_h repr)
{
	RETV_IF(NULL == repr, 0);
	RETV_IF(NULL == repr->hash_table, 0);

	return g_hash_table_size(repr->hash_table);
}

static int _icl_repr_get_res_type_fn(const char *res_type, void *user_data)
{
	JsonArray *rt_array = user_data;
	json_array_add_string_element(rt_array, res_type);

	return IOTCON_FUNC_CONTINUE;
}

/*
 * A general result : {"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}
 */
static JsonObject* _icl_repr_data_generate_json(iotcon_repr_h cur_repr,
		unsigned int child_index)
{
	int i, ret, ifaces;
	char *iface_str;
	const char *uri_path;
	JsonObject *repr_obj = NULL;
	unsigned int rt_count = 0;
	JsonObject *prop_obj = NULL;
	iotcon_resource_types_h resource_types = NULL;

	RETV_IF(NULL == cur_repr, NULL);

	if (0 < iotcon_repr_get_keys_count(cur_repr)) {
		repr_obj = icl_obj_to_json(cur_repr);
		if (NULL == repr_obj) {
			ERR("icl_obj_to_json() Fail");
			json_object_unref(repr_obj);
			return NULL;
		}
	} else {
		repr_obj = json_object_new();
	}

	if (cur_repr->uri_path) {
		iotcon_repr_get_uri_path(cur_repr, &uri_path);
		json_object_set_string_member(repr_obj, IC_JSON_KEY_URI_PATH, uri_path);
	}

	if (cur_repr->res_types)
		rt_count = icl_resource_types_get_length(cur_repr->res_types);

	if (0 < rt_count || IOTCON_INTERFACE_NONE != cur_repr->interfaces) {
		prop_obj = json_object_new();
		json_object_set_object_member(repr_obj, IC_JSON_KEY_PROPERTY, prop_obj);
	}

	if (0 < rt_count) {
		JsonArray *rt_array = json_array_new();

		ret = iotcon_repr_get_resource_types(cur_repr, &resource_types);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_repr_get_resource_types() Fail(%d)", ret);
			json_object_unref(repr_obj);
			return NULL;
		}

		ret = iotcon_resource_types_foreach(resource_types, _icl_repr_get_res_type_fn,
				rt_array);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_types_foreach() Fail");
			json_object_unref(repr_obj);
			return NULL;
		}
		json_object_set_array_member(prop_obj, IC_JSON_KEY_RESOURCETYPES, rt_array);
	}

	if (IOTCON_INTERFACE_NONE != cur_repr->interfaces) {
		JsonArray *if_array = json_array_new();
		ifaces = iotcon_repr_get_resource_interfaces(cur_repr);
		for (i = 1; i <= ICL_INTERFACE_MAX; i = i << 1) {
			if (IOTCON_INTERFACE_NONE == (ifaces & i)) /* this interface not exist */
				continue;
			ret = ic_utils_convert_interface_flag((ifaces & i), &iface_str);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("ic_utils_convert_interface_flag(%d) Fail(%d)", i, ret);
				json_object_unref(repr_obj);
				json_array_unref(if_array);
				return NULL;
			}
			json_array_add_string_element(if_array, iface_str);
		}

		json_object_set_array_member(prop_obj, IC_JSON_KEY_INTERFACES, if_array);
	}

	return repr_obj;
}

static JsonObject* _icl_repr_data_generate_parent(iotcon_repr_h cur_repr,
		unsigned int child_index)
{
	FN_CALL;
	JsonObject *obj = _icl_repr_data_generate_json(cur_repr, child_index);
	if (NULL == obj) {
		ERR("_icl_repr_data_generate_json() Fail");
		return NULL;
	}

	return obj;
}

static JsonObject* _icl_repr_data_generate_child(iotcon_repr_h cur_repr,
		unsigned int child_index)
{
	FN_CALL;
	JsonObject *obj = _icl_repr_data_generate_json(cur_repr, child_index);
	if (NULL == obj) {
		ERR("_icl_repr_data_generate_json() Fail");
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
static JsonObject* _icl_repr_generate_json(iotcon_repr_h repr)
{
	unsigned int child_index;
	unsigned int child_count = 0;
	JsonArray *root_array = NULL;
	JsonObject *repr_obj, *root_obj;
	iotcon_repr_h child_repr = NULL;

	RETV_IF(NULL == repr, NULL);

	root_obj = json_object_new();
	root_array = json_array_new();

	if (repr->children)
		child_count = iotcon_repr_get_children_count(repr);

	repr_obj = _icl_repr_data_generate_parent(repr, child_index);
	if (NULL == repr_obj) {
		ERR("_icl_repr_data_generate_parent() Fali(NULL == repr_obj)");
		json_object_unref(root_obj);
		json_array_unref(root_array);
		return NULL;
	}
	json_array_add_object_element(root_array, repr_obj);

	for (child_index = 0; child_index < child_count; child_index++) {
		iotcon_repr_get_nth_child(repr, child_index, &child_repr);
		repr_obj = _icl_repr_data_generate_child(child_repr, child_index);
		if (NULL == repr_obj) {
			ERR("_icl_repr_data_generate_child() Fali(NULL == repr_obj)");
			json_object_unref(root_obj);
			json_array_unref(root_array);
			return NULL;
		}
		json_array_add_object_element(root_array, repr_obj);
	}

	json_object_set_array_member(root_obj, IC_JSON_KEY_OC, root_array);

	return root_obj;
}

/*
 * returned string SHOULD be released by you
 */
gchar* _icl_repr_obj_to_json(JsonObject *obj, bool set_pretty)
{
	gchar *json_data;
	JsonNode *root_node = NULL;
	JsonGenerator *gen = json_generator_new();
#if JSON_CHECK_VERSION(0,14,0)
	json_generator_set_pretty(gen, set_pretty);
#endif

	root_node = json_node_new(JSON_NODE_OBJECT);
	json_node_set_object(root_node, obj);
	json_generator_set_root(gen, root_node);

	json_data = json_generator_to_data(gen, NULL);
	DBG("result : %s", json_data);

	json_node_free(root_node);
	g_object_unref(gen);

	return json_data;
}


char* icl_repr_generate_json(iotcon_repr_h repr, bool set_pretty)
{
	char *json_data;
	JsonObject *obj;

	obj = _icl_repr_generate_json(repr);
	if (NULL == obj) {
		ERR("icl_repr_generate_json() Fail");
		return NULL;
	}

	json_data = _icl_repr_obj_to_json(obj, set_pretty);
	if (NULL == json_data) {
		ERR("_icl_repr_obj_to_json() Fail");
		return NULL;
	}

	return json_data;
}


/*
 * returned string SHOULD be released by you
 */
char* icl_repr_json_get_uri_path(const char *json_string)
{
	GError *error = NULL;
	gboolean ret = FALSE;
	char *uri_path = NULL;
	const char *str_value = NULL;
	JsonParser *parser;
	JsonObject *root_obj;

	RETV_IF(NULL == json_string, NULL);

	parser = json_parser_new();
	ret = json_parser_load_from_data(parser, json_string, strlen(json_string), &error);
	if (FALSE == ret) {
		ERR("json_parser_load_from_data() Fail(%s)", error->message);
		g_error_free(error);
		g_object_unref(parser);
		return NULL;
	}

	root_obj = json_node_get_object(json_parser_get_root(parser));
	if (json_object_has_member(root_obj, IC_JSON_KEY_URI_PATH)) {
		str_value = json_object_get_string_member(root_obj, IC_JSON_KEY_URI_PATH);
		if (NULL == str_value) {
			ERR("json_object_get_string_member() Fail");
			return NULL;
		}
		uri_path = strdup(str_value);
		if (NULL == uri_path) {
			ERR("strdup() Fail");
			return NULL;
		}
	}
	return uri_path;
}


/*
 * A general input : {"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}
 *
 * Result : A iotcon_repr_h handle including uri_path
 */
static iotcon_repr_h _icl_repr_create_repr(JsonObject *rsrc_obj)
{
	FN_CALL;
	char *json_data;
	iotcon_repr_h repr;

	json_data = _icl_repr_obj_to_json(rsrc_obj, false);
	if (NULL == json_data) {
		ERR("json_data is NULL");
		return NULL;
	}

	repr = icl_repr_parse_json(json_data);
	if (NULL == repr) {
		ERR("icl_repr_parse_json() Fail");
		g_free(json_data);
		return NULL;
	}

	free(json_data);

	return repr;
}


/*
 * A general input : {oc:[{"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}},
 * 						{"href":"/a/child","rep":{"string":"World","double_val":5.7},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}]}
 *
 * Result : iotcon_repr_h handles(parent and child) including uri_path
 */
iotcon_repr_h icl_repr_create_repr(const char *json_string)
{
	FN_CALL;
	gboolean ret;
	JsonParser *parser;
	GError *error = NULL;
	JsonArray *rsrc_array;
	iotcon_repr_h repr_cur;
	iotcon_repr_h repr_parent = NULL;
	JsonObject *root_obj, *rsrc_obj;
	unsigned int rsrc_count, rsrc_index;

	parser = json_parser_new();
	ret = json_parser_load_from_data(parser, json_string, strlen(json_string), &error);
	if (FALSE == ret) {
		ERR("json_parser_load_from_data() Fail(%s)", error->message);
		g_error_free(error);
		return NULL;
	}

	root_obj = json_node_get_object(json_parser_get_root(parser));

	/* parse 'oc' prefix */
	rsrc_array = json_object_get_array_member(root_obj, IC_JSON_KEY_OC);
	if (NULL == rsrc_array) {
		ERR("json_object_get_array_member() Fail");
		return NULL;
	}

	rsrc_count = json_array_get_length(rsrc_array);
	for (rsrc_index = 0; rsrc_index < rsrc_count; rsrc_index++) {
		rsrc_obj = json_array_get_object_element(rsrc_array, rsrc_index);
		repr_cur = _icl_repr_create_repr(rsrc_obj);
		if (NULL == repr_cur) {
			ERR("_icl_repr_create_repr() Fail");
			if (0 < rsrc_index) /* parent was already made */
				iotcon_repr_free(repr_parent);
			return NULL;
		}

		if (0 == rsrc_index) { /* parent representation */
			repr_parent = repr_cur;
		} else { /* child representation */
			ret = iotcon_repr_append_child(repr_parent, repr_cur);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_repr_append_child() Fail(%d)", ret);
				iotcon_repr_free(repr_parent);
				return NULL;
			}
		}
	}

	return repr_parent;
}


static int _icl_repr_convert_interface_string(const char *src, iotcon_interface_e *dest)
{
	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	if (IC_STR_EQUAL == strcmp(IC_INTERFACE_DEFAULT, src)) {
		*dest = IOTCON_INTERFACE_DEFAULT;
	} else if (IC_STR_EQUAL == strcmp(IC_INTERFACE_LINK, src)) {
		*dest = IOTCON_INTERFACE_LINK;
	} else if (IC_STR_EQUAL == strcmp(IC_INTERFACE_BATCH, src)) {
		*dest = IOTCON_INTERFACE_BATCH;
	} else {
		ERR("Invalid Interface");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


int icl_repr_parse_resource_property(JsonObject *prop_obj,
		iotcon_resource_types_h *types, int *ifaces)
{
	int ret;
	int ret_ifaces = IOTCON_INTERFACE_NONE;
	JsonArray *iface_array, *rtye_array;
	iotcon_resource_types_h res_types = NULL;

	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	rtye_array = json_object_get_array_member(prop_obj, IC_JSON_KEY_RESOURCETYPES);
	if (rtye_array) {
		unsigned int rt_count, rt_index;

		rt_count = json_array_get_length(rtye_array);
		if (0 < rt_count) {
			res_types = iotcon_resource_types_new();
			if (NULL == res_types) {
				ERR("iotcon_resource_types_new() Fail");
				return IOTCON_ERROR_OUT_OF_MEMORY;
			}

			for (rt_index = 0; rt_index < rt_count; rt_index++) {
				const char *rtype_str;

				rtype_str = json_array_get_string_element(rtye_array, rt_index);
				iotcon_resource_types_insert(res_types, rtype_str);
			}
		}
	}

	iface_array = json_object_get_array_member(prop_obj, IC_JSON_KEY_INTERFACES);
	if (iface_array) {
		iotcon_interface_e iface_flag;
		unsigned int if_count, if_index;

		if_count = json_array_get_length(iface_array);
		for (if_index = 0; if_index < if_count; if_index++) {
			const char *iface_str = json_array_get_string_element(iface_array, if_index);
			ret = _icl_repr_convert_interface_string(iface_str, &iface_flag);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_ioty_convert_interface_string() Fail(%d)", ret);
				if (res_types)
					iotcon_resource_types_free(res_types);
				return ret;
			}

			ret_ifaces |= iface_flag;
		}
	}

	*types = res_types;
	*ifaces = ret_ifaces;

	return IOTCON_ERROR_NONE;
}


/*
 * A general input : {"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}
 */
iotcon_repr_h icl_repr_parse_json(const char *json_string)
{
	int ret;
	JsonParser *parser;
	GError *error = NULL;
	const char *str_value;
	iotcon_repr_h repr = NULL;
	iotcon_resource_types_h res_types;
	int ifaces = IOTCON_INTERFACE_NONE;
	JsonObject *root_obj, *property_obj;

	RETV_IF(NULL == json_string, NULL);

	DBG("input str : %s", json_string);

	parser = json_parser_new();
	ret = json_parser_load_from_data(parser, json_string, strlen(json_string), &error);
	if (FALSE == ret) {
		ERR("json_parser_load_from_data() Fail(%s)", error->message);
		g_error_free(error);
		g_object_unref(parser);
		return NULL;
	}

	root_obj = json_node_get_object(json_parser_get_root(parser));
	if (NULL == root_obj) {
		ERR("json_node_get_object() Fail");
		g_object_unref(parser);
		return NULL;
	}

	repr = icl_obj_from_json(root_obj);
	if (NULL == repr) {
		ERR("icl_obj_from_json() Fail()");
		g_object_unref(parser);
		return NULL;
	}

	str_value = json_object_get_string_member(root_obj, IC_JSON_KEY_URI_PATH);
	ret = iotcon_repr_set_uri_path(repr, str_value);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_repr_set_uri_path() Fail(%d)", ret);
		iotcon_repr_free(repr);
		g_object_unref(parser);
		return NULL;
	}

	property_obj = json_object_get_object_member(root_obj, IC_JSON_KEY_PROPERTY);
	if (NULL == property_obj) {
		/* Here is normal condition */
		g_object_unref(parser);
		return repr;
	}

	ret = icl_repr_parse_resource_property(property_obj, &res_types, &ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_repr_parse_resource_property() Fail(%d)", ret);
		iotcon_repr_free(repr);
		g_object_unref(parser);
		return NULL;
	}

	iotcon_repr_set_resource_types(repr, res_types);
	iotcon_repr_set_resource_interfaces(repr, ifaces);

	g_object_unref(parser);

	return repr;
}


API void iotcon_repr_free(iotcon_repr_h repr)
{
	FN_CALL;
	RET_IF(NULL == repr);

	if (false == _icl_repr_dec_ref_count(repr))
		return;

	free(repr->uri_path);

	/* (GDestroyNotify) : iotcon_repr_h is proper type than gpointer */
	g_list_free_full(repr->children, (GDestroyNotify)iotcon_repr_free);

	/* null COULD be allowed */
	if (repr->res_types)
		iotcon_resource_types_free(repr->res_types);
	g_hash_table_destroy(repr->hash_table);
	free(repr);
}

static void _icl_repr_obj_clone(char *key, iotcon_value_h src_val, iotcon_repr_h dest_repr)
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
		copied_val = icl_value_clone(src_val);
		if (NULL == copied_val) {
			ERR("icl_value_clone() Fail");
			return;
		}

		icl_obj_set_value(dest_repr, key, copied_val);
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

		value = icl_value_new_list(copied_list);
		if (NULL == value) {
			ERR("icl_value_new_list() Fail");
			iotcon_list_free(copied_list);
			return;
		}

		icl_obj_set_value(dest_repr, key, value);
		break;
	case IOTCON_TYPE_REPR:
		ret = icl_value_get_repr(src_val, &child_repr);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_value_get_repr() Fail(%d)", ret);
			return;
		}

		copied_repr = iotcon_repr_clone(child_repr);
		if (NULL == copied_repr) {
			ERR("icl_list_clone() Fail");
			return;
		}

		value = icl_value_new_repr(copied_repr);
		if (NULL == value) {
			ERR("icl_value_new_repr(%p) Fail", copied_repr);
			return;
		}

		icl_obj_set_value(dest_repr, key, value);
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
	iotcon_resource_types_h list;

	RETV_IF(NULL == src, NULL);

	dest = iotcon_repr_new();
	if (NULL == dest) {
		ERR("iotcon_repr_new() Fail");
		return NULL;
	}

	if (src->uri_path) {
		dest->uri_path = strdup(src->uri_path);
		if (NULL == dest->uri_path) {
			ERR("strdup() Fail");
			iotcon_repr_free(dest);
			return NULL;
		}
	}

	if (src->interfaces)
		dest->interfaces = src->interfaces;

	if (src->res_types) {
		list = iotcon_resource_types_clone(src->res_types);
		if (NULL == list) {
			ERR("iotcon_resource_types_clone() Fail");
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

	g_hash_table_foreach(src->hash_table, (GHFunc)_icl_repr_obj_clone, dest);

	return dest;
}

API char* iotcon_repr_generate_json(iotcon_repr_h repr)
{
	return icl_repr_generate_json(repr, TRUE);
}
