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
#include "icl-representation.h"
#include "icl-state.h"
#include "icl-list.h"
#include "icl-value.h"
#include "icl-resource-types.h"
#include "icl-resource-interfaces.h"
#include "icl-response.h"
#include "icl-payload.h"

static GVariant* _icl_state_value_to_gvariant(GHashTable *hash);
static iotcon_list_h _icl_state_list_from_gvariant(GVariant *var);

static GVariant* _icl_state_list_to_gvariant(iotcon_list_h list)
{
	GList *node;
	GVariant *var = NULL;
	GVariantBuilder builder;
	iotcon_state_h state;
	struct icl_value_s *state_value = NULL;

	RETV_IF(NULL == list, NULL);

	g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

	switch (list->type) {
	case IOTCON_TYPE_INT:
		for (node = list->list; node; node = node->next) {
			state_value = node->data;
			g_variant_builder_add(&builder, "i", ((icl_basic_s*)state_value)->val.i);
		}
		break;
	case IOTCON_TYPE_BOOL:
		for (node = list->list; node; node = node->next) {
			state_value = node->data;
			g_variant_builder_add(&builder, "b", ((icl_basic_s*)state_value)->val.b);
		}
		break;
	case IOTCON_TYPE_DOUBLE:
		for (node = list->list; node; node = node->next) {
			state_value = node->data;
			g_variant_builder_add(&builder, "d", ((icl_basic_s*)state_value)->val.d);
		}
		break;
	case IOTCON_TYPE_STR:
		for (node = list->list; node; node = node->next) {
			state_value = node->data;
			g_variant_builder_add(&builder, "s", ((icl_basic_s*)state_value)->val.s);
		}
		break;
	case IOTCON_TYPE_BYTE_STR:
		for (node = list->list; node; node = node->next) {
			state_value = node->data;
			var = g_variant_new_fixed_array(G_VARIANT_TYPE("y"),
					((icl_val_byte_str_s*)state_value)->s,
					((icl_val_byte_str_s*)state_value)->len,
					sizeof(unsigned char));
			g_variant_builder_add(&builder, "v", var);
		}
		break;
	case IOTCON_TYPE_LIST:
		for (node = list->list; node; node = node->next) {
			state_value = node->data;
			var = _icl_state_list_to_gvariant(((icl_val_list_s*)state_value)->list);
			g_variant_builder_add(&builder, "v", var);
		}
		break;
	case IOTCON_TYPE_STATE:
		for (node = list->list; node; node = node->next) {
			state_value = node->data;
			state = ((icl_val_state_s*)state_value)->state;
			var = _icl_state_value_to_gvariant(state->hash_table);
			g_variant_builder_add(&builder, "v", var);
		}
		break;
	default:
		ERR("Invalid type(%d)", list->type);
		return NULL;
	}

	return g_variant_builder_end(&builder);

}


static GVariantBuilder* _icl_state_value_to_gvariant_builder(GHashTable *hash)
{
	gpointer key, value;
	GHashTableIter iter;
	GVariant *var = NULL;
	iotcon_state_h state;
	GVariantBuilder *builder;
	struct icl_value_s *state_value = NULL;

	if (NULL == hash)
		return NULL;

	builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));

	g_hash_table_iter_init(&iter, hash);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		state_value = value;
		switch (state_value->type) {
		case IOTCON_TYPE_INT:
			var = g_variant_new_int32(((icl_basic_s*)state_value)->val.i);
			break;
		case IOTCON_TYPE_BOOL:
			var = g_variant_new_boolean(((icl_basic_s*)state_value)->val.b);
			break;
		case IOTCON_TYPE_DOUBLE:
			var = g_variant_new_double(((icl_basic_s*)state_value)->val.d);
			break;
		case IOTCON_TYPE_STR:
			var = g_variant_new_string(((icl_basic_s*)state_value)->val.s);
			break;
		case IOTCON_TYPE_NULL:
			var = g_variant_new_string(IC_STR_NULL);
			break;
		case IOTCON_TYPE_BYTE_STR:
			var = g_variant_new_fixed_array(G_VARIANT_TYPE("y"),
					((icl_val_byte_str_s*)state_value)->s,
					((icl_val_byte_str_s*)state_value)->len,
					sizeof(unsigned char));
			break;
		case IOTCON_TYPE_LIST:
			var = _icl_state_list_to_gvariant(((icl_val_list_s*)state_value)->list);
			break;
		case IOTCON_TYPE_STATE:
			state = ((icl_val_state_s*)state_value)->state;
			var = _icl_state_value_to_gvariant(state->hash_table);
			break;
		case IOTCON_TYPE_NONE:
		default:
			ERR("Invalid Type(%d)", state_value->type);
			g_variant_builder_unref(builder);
			return NULL;
		}

		if (var)
			g_variant_builder_add(builder, "{sv}", key, var);
	}

	return builder;
}


static GVariant* _icl_state_value_to_gvariant(GHashTable *hash)
{
	GVariantBuilder* builder;

	builder = _icl_state_value_to_gvariant_builder(hash);

	return g_variant_builder_end(builder);
}


static GVariant* _icl_representation_empty_gvariant(void)
{
	GVariant *value;
	GVariantBuilder types, ifaces, repr, children;

	g_variant_builder_init(&ifaces, G_VARIANT_TYPE("as"));
	g_variant_builder_init(&types, G_VARIANT_TYPE("as"));
	g_variant_builder_init(&repr, G_VARIANT_TYPE("a{sv}"));
	g_variant_builder_init(&children, G_VARIANT_TYPE("av"));

	value = g_variant_new("(sasasa{sv}av)", IC_STR_NULL, &ifaces, &types, &repr, &children);

	return value;
}


GVariant* icl_representation_to_gvariant(iotcon_representation_h repr)
{
	GList *node;
	const char *uri_path;
	GVariant *value, *child;
	iotcon_state_h state;
	GVariantBuilder *repr_gvar = NULL;
	GVariantBuilder children, resource_types, resource_ifaces;

	if (NULL == repr)
		return _icl_representation_empty_gvariant();

	/* uri path */
	uri_path = ic_utils_dbus_encode_str(repr->uri_path);

	/* Resource Types & Interfaces */
	g_variant_builder_init(&resource_types, G_VARIANT_TYPE("as"));
	g_variant_builder_init(&resource_ifaces, G_VARIANT_TYPE("as"));

	if (ICL_VISIBILITY_PROP & repr->visibility) {
		if (repr->res_types) {
			for (node = repr->res_types->type_list; node; node = node->next)
				g_variant_builder_add(&resource_types, "s", node->data);
		}

		if (repr->interfaces) {
			for (node = repr->interfaces->iface_list; node; node = node->next)
				g_variant_builder_add(&resource_ifaces, "s", node->data);
		}
	}

	/* Representation */
	state = repr->state;
	if (state && (ICL_VISIBILITY_REPR & repr->visibility))
		repr_gvar = _icl_state_value_to_gvariant_builder(state->hash_table);

	/* Children */
	g_variant_builder_init(&children, G_VARIANT_TYPE("av"));

	for (node = repr->children; node; node = node->next) {
		/* generate recursively */
		child = icl_representation_to_gvariant(node->data);
		g_variant_builder_add(&children, "v", child);
	}

	value = g_variant_new("(sasasa{sv}av)", uri_path, &resource_ifaces, &resource_types,
			repr_gvar, &children);

	return value;
}


void icl_state_from_gvariant(iotcon_state_h state, GVariantIter *iter)
{
	char *key;
	GVariant *var;
	const char *str_value;
	iotcon_list_h list_value;
	iotcon_value_h value = NULL;
	iotcon_state_h state_value = NULL;

	while (g_variant_iter_loop(iter, "{sv}", &key, &var)) {
		if (g_variant_is_of_type(var, G_VARIANT_TYPE_BOOLEAN)) {
			value = icl_value_create_bool(g_variant_get_boolean(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_INT32)) {
			value = icl_value_create_int(g_variant_get_int32(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_DOUBLE)) {
			value = icl_value_create_double(g_variant_get_double(var));

		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_STRING)) {
			str_value = g_variant_get_string(var, NULL);
			if (IC_STR_EQUAL == strcmp(IC_STR_NULL, str_value))
				value = icl_value_create_null();
			else
				value = icl_value_create_str(str_value);
		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE("a{sv}"))) {
			GVariantIter state_iter;
			iotcon_state_create(&state_value);
			g_variant_iter_init(&state_iter, var);
			icl_state_from_gvariant(state_value, &state_iter);
			value = icl_value_create_state(state_value);
		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE("ay"))) {
			value = icl_value_create_byte_str(g_variant_get_data(var),
					g_variant_get_size(var));
		} else if (g_variant_is_of_type(var, G_VARIANT_TYPE_ARRAY)) {
			list_value = _icl_state_list_from_gvariant(var);
			value = icl_value_create_list(list_value);
		} else {
			ERR("Invalid type(%s)", g_variant_get_type_string(var));
		}
		g_hash_table_replace(state->hash_table, ic_utils_strdup(key), value);
	}

	return;
}


static iotcon_list_h _icl_state_list_from_gvariant(GVariant *var)
{
	int ret;
	GVariantIter iter;
	const GVariantType *type;
	iotcon_list_h list = NULL;

	type = g_variant_get_type(var);

	g_variant_iter_init(&iter, var);

	if (g_variant_type_equal(G_VARIANT_TYPE("ab"), type)) {
		bool b;
		ret = iotcon_list_create(IOTCON_TYPE_BOOL, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create() Fail(%d)", ret);
			return NULL;
		}

		while (g_variant_iter_loop(&iter, "b", &b))
			iotcon_list_add_bool(list, b, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("ai"), type)) {
		int i;
		ret = iotcon_list_create(IOTCON_TYPE_INT, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create() Fail(%d)", ret);
			return NULL;
		}

		while (g_variant_iter_loop(&iter, "i", &i))
			iotcon_list_add_int(list, i, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("ad"), type)) {
		double d;
		ret = iotcon_list_create(IOTCON_TYPE_DOUBLE, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create() Fail(%d)", ret);
			return NULL;
		}

		while (g_variant_iter_loop(&iter, "d", &d))
			iotcon_list_add_double(list, d, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("as"), type)) {
		char *s;
		ret = iotcon_list_create(IOTCON_TYPE_STR, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create() Fail(%d)", ret);
			return NULL;
		}

		while (g_variant_iter_loop(&iter, "s", &s))
			iotcon_list_add_str(list, s, -1);
	} else if (g_variant_type_equal(G_VARIANT_TYPE("av"), type)) {
		GVariant *variant;
		iotcon_list_h list_value;
		iotcon_state_h state_value = NULL;

		while (g_variant_iter_loop(&iter, "v", &variant)) {
			if (g_variant_is_of_type(variant, G_VARIANT_TYPE("a{sv}"))) {
				GVariantIter state_iter;
				if (NULL == list) {
					ret = iotcon_list_create(IOTCON_TYPE_STATE, &list);
					if (IOTCON_ERROR_NONE != ret) {
						ERR("iotcon_list_create() Fail(%d)", ret);
						return NULL;
					}
				}
				iotcon_state_create(&state_value);
				g_variant_iter_init(&state_iter, variant);
				icl_state_from_gvariant(state_value, &state_iter);
				iotcon_list_add_state(list, state_value, -1);
			} else if (g_variant_is_of_type(variant, G_VARIANT_TYPE("ay"))) {
				unsigned char *byte_str;
				if (NULL == list) {
					ret = iotcon_list_create(IOTCON_TYPE_BYTE_STR, &list);
					if (IOTCON_ERROR_NONE != ret) {
						ERR("iotcon_list_create() Fail(%d)", ret);
						return NULL;
					}
				}
				byte_str = (unsigned char*)g_variant_get_data(variant);
				iotcon_list_add_byte_str(list, byte_str, g_variant_get_size(variant), -1);
			} else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_ARRAY)) {
				if (NULL == list) {
					ret = iotcon_list_create(IOTCON_TYPE_LIST, &list);
					if (IOTCON_ERROR_NONE != ret) {
						ERR("iotcon_list_create() Fail(%d)", ret);
						return NULL;
					}
				}
				list_value = _icl_state_list_from_gvariant(variant);
				iotcon_list_add_list(list, list_value, -1);
			} else {
				ERR("Invalid type(%s)", g_variant_get_type_string(variant));
			}
		}
	} else {
		ERR("Invalid type(%s)", g_variant_get_type_string(var));
	}

	return list;
}


iotcon_representation_h icl_representation_from_gvariant(GVariant *var)
{
	int ret;
	GVariant *child;
	iotcon_representation_h repr;
	iotcon_state_h state;
	char *uri_path, *resource_type, *resource_iface;
	GVariantIter *children, *repr_gvar, *resource_types, *resource_ifaces;

	ret = iotcon_representation_create(&repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return NULL;
	}

	ret = iotcon_state_create(&state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return NULL;
	}

	g_variant_get(var, "(&sasasa{sv}av)", &uri_path, &resource_ifaces, &resource_types,
			&repr_gvar, &children);

	/* uri path */
	if (IC_STR_EQUAL != strcmp(IC_STR_NULL, uri_path))
		repr->uri_path = strdup(uri_path);

	/* resource types */
	if (g_variant_iter_n_children(resource_types)) {
		ret = iotcon_resource_types_create(&repr->res_types);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_types_create() Fail(%d)", ret);
			g_variant_iter_free(resource_ifaces);
			g_variant_iter_free(resource_types);
			g_variant_iter_free(children);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(repr);
			return NULL;
		}

		while (g_variant_iter_loop(resource_types, "s", &resource_type))
			iotcon_resource_types_add(repr->res_types, resource_type);
	}
	g_variant_iter_free(resource_types);

	/* resource ifaces */
	if (g_variant_iter_n_children(resource_ifaces)) {
		ret = iotcon_resource_interfaces_create(&repr->interfaces);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_interfaces_create() Fail(%d)", ret);
			g_variant_iter_free(resource_ifaces);
			g_variant_iter_free(children);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(repr);
			return NULL;
		}

		while (g_variant_iter_loop(resource_ifaces, "s", &resource_iface))
			iotcon_resource_interfaces_add(repr->interfaces, resource_iface);
	}
	g_variant_iter_free(resource_ifaces);

	/* attribute */
	icl_state_from_gvariant(state, repr_gvar);

	ret = iotcon_representation_set_state(repr, state);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_state() Fail(%d)", ret);
		iotcon_state_destroy(state);
		iotcon_representation_destroy(repr);
		return NULL;
	}
	iotcon_state_destroy(state);

	/* children */
	while (g_variant_iter_loop(children, "v", &child)) {
		repr->children = g_list_append(repr->children,
				icl_representation_from_gvariant(child));
	}
	g_variant_iter_free(children);

	return repr;
}
