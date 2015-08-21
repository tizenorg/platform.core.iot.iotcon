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
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-repr.h"
#include "icl-repr-list.h"
#include "icl-repr-value.h"
#include "icl-resource-types.h"
#include "icl-response.h"
#include "icl-payload.h"

static GVariantBuilder* _icl_repr_value_to_gvariant(GHashTable *hash);

static GVariant* _icl_repr_list_to_gvariant(iotcon_list_h list)
{
	GList *node;
	GVariant *var = NULL;
	GVariantBuilder builder;
	struct icl_value_s *repr_value = NULL;

	RETV_IF(NULL == list, NULL);

	g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

	switch (list->type) {
	case IOTCON_TYPE_INT:
		for (node = list->list; node; node = node->next) {
			repr_value = node->data;
			g_variant_builder_add(&builder, "i", ((icl_basic_s*)repr_value)->val.i);
		}
		break;
	case IOTCON_TYPE_BOOL:
		for (node = list->list; node; node = node->next) {
			repr_value = node->data;
			g_variant_builder_add(&builder, "b", ((icl_basic_s*)repr_value)->val.b);
		}
		break;
	case IOTCON_TYPE_DOUBLE:
		for (node = list->list; node; node = node->next) {
			repr_value = node->data;
			g_variant_builder_add(&builder, "d", ((icl_basic_s*)repr_value)->val.d);
		}
		break;
	case IOTCON_TYPE_STR:
		for (node = list->list; node; node = node->next) {
			repr_value = node->data;
			g_variant_builder_add(&builder, "s", ((icl_basic_s*)repr_value)->val.s);
		}
		break;
	case IOTCON_TYPE_LIST:
		for (node = list->list; node; node = node->next) {
			repr_value = node->data;
			var = _icl_repr_list_to_gvariant(((icl_val_list_s*)repr_value)->list);
			g_variant_builder_add(&builder, "v", var);
		}
		break;
	case IOTCON_TYPE_REPR:
		for (node = list->list; node; node = node->next) {
			repr_value = node->data;
			var = icl_repr_to_gvariant(((icl_val_repr_s*)repr_value)->repr);
			g_variant_builder_add(&builder, "v", var);
		}
		break;
	default:
		ERR("Invalid type(%d)", list->type);
		return NULL;
	}

	return g_variant_builder_end(&builder);

}

static GVariantBuilder* _icl_repr_value_to_gvariant(GHashTable *hash)
{
	gpointer key, value;
	GHashTableIter iter;
	GVariant *var = NULL;
	GVariantBuilder *builder;
	struct icl_value_s *repr_value = NULL;

	if (NULL == hash)
		return NULL;

	builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

	g_hash_table_iter_init(&iter, hash);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		repr_value = value;
		switch (repr_value->type) {
		case IOTCON_TYPE_INT:
			var = g_variant_new_int32(((icl_basic_s*)repr_value)->val.i);
			break;
		case IOTCON_TYPE_BOOL:
			var = g_variant_new_boolean(((icl_basic_s*)repr_value)->val.b);
			break;
		case IOTCON_TYPE_DOUBLE:
			var = g_variant_new_double(((icl_basic_s*)repr_value)->val.d);
			break;
		case IOTCON_TYPE_STR:
			var = g_variant_new_string(((icl_basic_s*)repr_value)->val.s);
			break;
		case IOTCON_TYPE_NULL:
			var = g_variant_new_string(IC_STR_NULL);
			break;
		case IOTCON_TYPE_LIST:
			var = _icl_repr_list_to_gvariant(((icl_val_list_s*)repr_value)->list);
			break;
		case IOTCON_TYPE_REPR:
			var = icl_repr_to_gvariant(((icl_val_repr_s*)repr_value)->repr);
			break;
		case IOTCON_TYPE_NONE:
		default:
			ERR("Invalid Type");
			g_variant_builder_unref(builder);
			return NULL;
		}

		if (var)
			g_variant_builder_add(builder, "{sv}", key, var);
	}

	return builder;
}


GVariant* icl_repr_to_gvariant(iotcon_repr_h repr)
{
	GList *node;
	int ifaces = 0;
	const char *uri_path;
	GVariant *value, *child;
	GVariantBuilder *repr_gvar = NULL;
	GVariantBuilder children, resource_types;

	RETV_IF(NULL == repr, NULL);

	/* uri path */
	uri_path = ic_utils_dbus_encode_str(repr->uri_path);

	/* Resource Types & Interfaces */
	g_variant_builder_init(&resource_types, G_VARIANT_TYPE("as"));

	if (ICL_VISIBILITY_PROP & repr->visibility) {
		if (repr->res_types) {
			for (node = repr->res_types->type_list; node; node = node->next)
				g_variant_builder_add(&resource_types, "s", node->data);
		}

		ifaces = repr->interfaces;
	}

	/* Representation */
	if (ICL_VISIBILITY_REPR & repr->visibility)
		repr_gvar = _icl_repr_value_to_gvariant(repr->hash_table);

	/* Children */
	g_variant_builder_init(&children, G_VARIANT_TYPE("av"));

	for (node = repr->children; node; node = node->next) {
		/* generate recursively */
		child = icl_repr_to_gvariant(node->data);
		g_variant_builder_add(&children, "v", child);
	}

	value = g_variant_new("(siasa{sv}av)", uri_path, ifaces, &resource_types,
			repr_gvar, &children);

	return value;
}


static iotcon_list_h _icl_repr_list_from_gvariant(GVariant *var)
{
	GVariantIter iter;
	const GVariantType *type;
	iotcon_list_h list = NULL;

	type = g_variant_get_type(var);

	g_variant_iter_init(&iter, var);

	if (g_variant_type_equal(G_VARIANT_TYPE("ab"), type)) {
		bool b;
		list = iotcon_list_new(IOTCON_TYPE_BOOL);
		while (g_variant_iter_loop(&iter, "b", &b))
			iotcon_list_insert_bool(list, b, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("ai"), type)) {
		int i;
		list = iotcon_list_new(IOTCON_TYPE_INT);
		while (g_variant_iter_loop(&iter, "i", &i))
			iotcon_list_insert_int(list, i, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("ad"), type)) {
		double d;
		list = iotcon_list_new(IOTCON_TYPE_DOUBLE);
		while (g_variant_iter_loop(&iter, "d", &d))
			iotcon_list_insert_double(list, d, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("as"), type)) {
		char *s;
		list = iotcon_list_new(IOTCON_TYPE_STR);
		while (g_variant_iter_loop(&iter, "s", &s))
			iotcon_list_insert_str(list, s, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("v"), type)) {
		GVariant *value;
		iotcon_list_h list_value;
		iotcon_repr_h repr_value;

		while (g_variant_iter_loop(&iter, "v", &value)) {
			if (g_variant_is_of_type(value, G_VARIANT_TYPE_ARRAY)) {
				list_value = _icl_repr_list_from_gvariant(value);
				iotcon_list_insert_list(list, list_value, -1);
			} else if (g_variant_is_of_type(value, G_VARIANT_TYPE("(siasa{sv}av)"))) {
				repr_value = icl_repr_from_gvariant(value);
				iotcon_list_insert_repr(list, repr_value, -1);
			}
		}
	}

	return list;
}


static void _icl_repr_value_from_gvariant(iotcon_repr_h repr, GVariantIter *iter)
{
	char *key;
	GVariant *var;
	const char *str_value;
	iotcon_list_h list_value;
	iotcon_repr_h repr_value;
	iotcon_value_h value = NULL;

	while (g_variant_iter_loop(iter, "{sv}", &key, &var)) {

		if (g_variant_is_of_type(var, G_VARIANT_TYPE_BOOLEAN)) {
			value = icl_value_new_bool(g_variant_get_boolean(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_INT32)) {
			value = icl_value_new_int(g_variant_get_int32(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_DOUBLE)) {
			value = icl_value_new_double(g_variant_get_double(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_STRING)) {
			str_value = g_variant_get_string(var, NULL);
			if (IC_STR_EQUAL == strcmp(IC_STR_NULL, str_value))
				value = icl_value_new_null();
			else
				value = icl_value_new_str(str_value);

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_ARRAY)) {
			list_value = _icl_repr_list_from_gvariant(var);
			value = icl_value_new_list(list_value);

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE("(siasa{sv}av)"))) {
			repr_value = icl_repr_from_gvariant(var);
			value = icl_value_new_repr(repr_value);
		}

		g_hash_table_replace(repr->hash_table, ic_utils_strdup(key), value);
	}

	return;
}


iotcon_repr_h icl_repr_from_gvariant(GVariant *var)
{
	iotcon_repr_h repr;
	GVariant *child;
	char *uri_path, *resource_type;
	GVariantIter *children, *repr_gvar, *resource_types;

	repr = iotcon_repr_new();
	if (NULL == repr) {
		ERR("iotcon_repr_new() Fail");
		return NULL;
	}

	DBG("repr : %s", g_variant_print(var, FALSE));

	g_variant_get(var, "(&siasa{sv}av)", &uri_path, &repr->interfaces, &resource_types,
			&repr_gvar, &children);

	/* uri path */
	if (IC_STR_EQUAL != strcmp(IC_STR_NULL, uri_path))
		repr->uri_path = strdup(uri_path);

	/* resource types */
	if (g_variant_iter_n_children(resource_types)) {
		repr->res_types = iotcon_resource_types_new();
		while (g_variant_iter_loop(resource_types, "s", &resource_type))
			iotcon_resource_types_insert(repr->res_types, resource_type);
	}
	g_variant_iter_free(resource_types);

	/* attribute */
	_icl_repr_value_from_gvariant(repr, repr_gvar);

	/* children */
	while (g_variant_iter_loop(children, "v", &child))
		repr->children = g_list_append(repr->children, icl_repr_from_gvariant(child));
	g_variant_iter_free(children);

	return repr;
}
