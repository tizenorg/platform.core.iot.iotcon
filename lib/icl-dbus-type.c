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
#include <glib.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource.h"
#include "icl-resource-types.h"
#include "icl-resource-interfaces.h"
#include "icl-options.h"
#include "icl-query.h"
#include "icl-request.h"
#include "icl-response.h"
#include "icl-remote-resource.h"
#include "icl-representation.h"
#include "icl-device.h"
#include "icl-payload.h"
#include "icl-observation.h"
#include "icl-dbus-type.h"

const char** icl_dbus_resource_ifaces_to_array(iotcon_resource_ifaces_h ifaces)
{
	int i = 0;
	GList *node = ifaces->iface_list;
	int len = g_list_length(node);

	const char **array = calloc(len + 1, sizeof(char*));
	if (NULL == array) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	for (node = ifaces->iface_list; node; node = node->next, i++)
		array[i] = node->data;
	array[i] = NULL;

	return array;
}


const char** icl_dbus_resource_types_to_array(iotcon_resource_types_h types)
{
	int i = 0;
	GList *node = types->type_list;
	int len = g_list_length(node);

	const char **array = calloc(len + 1, sizeof(char*));
	if (NULL == array) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	for (node = types->type_list; node; node = node->next, i++)
		array[i] = node->data;
	array[i] = NULL;

	return array;
}


GVariant* icl_dbus_representation_to_gvariant(struct icl_representation_s *repr)
{
	GVariantBuilder builder;
	GVariant *repr_gvar;

	g_variant_builder_init(&builder, G_VARIANT_TYPE("av"));

	if (repr) {
		repr_gvar = icl_representation_to_gvariant(repr);
		if (NULL == repr_gvar) {
			ERR("icl_representation_to_gvariant() Fail");
			g_variant_builder_clear(&builder);
			return NULL;
		}
		g_variant_builder_add(&builder, "v", repr_gvar);
	}

	return g_variant_builder_end(&builder);
}


GVariant* icl_dbus_response_to_gvariant(struct icl_resource_response *response)
{
	FN_CALL;
	GVariant *value;
	GVariant *repr_gvar;
	GHashTableIter iter;
	GVariantBuilder options;
	gpointer option_id, option_data;

	g_variant_builder_init(&options, G_VARIANT_TYPE("a(qs)"));
	if (response->header_options) {
		g_hash_table_iter_init(&iter, response->header_options->hash);
		while (g_hash_table_iter_next(&iter, &option_id, &option_data)) {
			g_variant_builder_add(&options, "(qs)", GPOINTER_TO_INT(option_id),
					option_data);
		}
	}

	repr_gvar = icl_representation_to_gvariant(response->repr);
	if (NULL == repr_gvar) {
		ERR("icl_representation_to_gvariant() Fail");
		g_variant_builder_clear(&options);
		return NULL;
	}

	value = g_variant_new("(a(qs)ivxx)",
			&options,
			response->result,
			repr_gvar,
			response->oic_request_h,
			response->oic_resource_h);

	return value;
}


GVariant* icl_dbus_remote_resource_to_gvariant(struct icl_remote_resource *resource)
{
	FN_CALL;
	bool is_secure;
	GVariant *value;
	GHashTableIter iter;
	GVariantBuilder options;
	gpointer option_id, option_data;

	g_variant_builder_init(&options, G_VARIANT_TYPE("a(qs)"));
	if (resource->header_options) {
		g_hash_table_iter_init(&iter, resource->header_options->hash);
		while (g_hash_table_iter_next(&iter, &option_id, &option_data)) {
			g_variant_builder_add(&options, "(qs)", GPOINTER_TO_INT(option_id),
					option_data);
		}
	}

	is_secure = resource->properties & IOTCON_RESOURCE_SECURE;
	value = g_variant_new("(ssba(qs)i)", resource->uri_path, resource->host_address,
			is_secure, &options, resource->connectivity_type);

	return value;
}


GVariant* icl_dbus_device_info_to_gvariant(iotcon_device_info_h device_info)
{
	GVariant *value;

	value = g_variant_new("(s)", device_info->device_name);

	return value;
}


GVariant* icl_dbus_platform_info_to_gvariant(iotcon_platform_info_h platform_info)
{
	GVariant *value;

	value = g_variant_new("(sssssssssss)",
			platform_info->platform_id,
			platform_info->manuf_name,
			platform_info->manuf_url,
			platform_info->model_number,
			platform_info->date_of_manuf,
			platform_info->platform_ver,
			platform_info->os_ver,
			platform_info->hardware_ver,
			platform_info->firmware_ver,
			platform_info->support_url,
			platform_info->system_time);

	return value;
}


GVariant* icl_dbus_query_to_gvariant(iotcon_query_h query)
{
	FN_CALL;
	gpointer key, value;
	GHashTableIter iter;
	GVariantBuilder builder;

	g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ss)"));
	if (query) {
		g_hash_table_iter_init(&iter, query->hash);
		while (g_hash_table_iter_next(&iter, &key, &value))
			g_variant_builder_add(&builder, "(ss)", key, value);
	}

	return g_variant_builder_end(&builder);
}


GVariant* icl_dbus_options_to_gvariant(iotcon_options_h options)
{
	GHashTableIter iter;
	GVariantBuilder builder;
	gpointer option_id, option_data;

	g_variant_builder_init(&builder, G_VARIANT_TYPE("a(qs)"));
	if (options) {
		g_hash_table_iter_init(&iter, options->hash);
		while (g_hash_table_iter_next(&iter, &option_id, &option_data)) {
			g_variant_builder_add(&builder, "(qs)", GPOINTER_TO_INT(option_id),
					option_data);
		}
	}

	return g_variant_builder_end(&builder);
}


GVariant* icl_dbus_observers_to_gvariant(iotcon_observers_h observers)
{
	GList *node;
	GVariantBuilder builder;

	g_variant_builder_init(&builder, G_VARIANT_TYPE("ai"));
	if (observers) {
		for (node = observers->observers_list; node; node = node->next)
			g_variant_builder_add(&builder, "i", GPOINTER_TO_INT(node->data));
	}

	return g_variant_builder_end(&builder);
}

