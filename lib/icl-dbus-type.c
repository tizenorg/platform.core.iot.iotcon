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
#include "icl-options.h"
#include "icl-query.h"
#include "icl-request.h"
#include "icl-response.h"
#include "icl-client.h"
#include "icl-repr.h"
#include "icl-dbus-type.h"
#include "icl-payload.h"

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


GVariant* icl_dbus_notimsg_to_gvariant(struct icl_notify_msg *msg)
{
	GVariantBuilder builder;
	GVariant *repr_gvar;

	g_variant_builder_init(&builder, G_VARIANT_TYPE("a(iv)"));

	if (msg) {
		repr_gvar = icl_repr_to_gvariant(msg->repr);
		if (NULL == repr_gvar) {
			ERR("icl_repr_to_gvariant() Fail");
			g_variant_builder_clear(&builder);
			return NULL;
		}
		g_variant_builder_add(&builder, "(iv)", msg->error_code, repr_gvar);
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

	repr_gvar = icl_repr_to_gvariant(response->repr);
	if (NULL == repr_gvar) {
		ERR("icl_repr_to_gvariant() Fail");
		g_variant_builder_clear(&options);
		return NULL;
	}

	value = g_variant_new("(sia(qs)ivii)",
			ic_utils_dbus_encode_str(response->new_uri_path),
			response->error_code,
			&options,
			response->result,
			repr_gvar,
			GPOINTER_TO_INT(response->request_handle),
			GPOINTER_TO_INT(response->resource_handle));

	DBG("response : %s", g_variant_print(value, FALSE));

	return value;
}


GVariant* icl_dbus_client_to_gvariant(struct icl_remote_resource *resource)
{
	FN_CALL;
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

	value = g_variant_new("(ssba(qs)i)", resource->uri_path, resource->host,
			resource->is_secure, &options, resource->conn_type);

	return value;
}


GVariant* icl_dbus_device_info_to_gvariant(const char *device_name)
{
	GVariant *value;

	value = g_variant_new("(s)", device_name);

	return value;
}


GVariant* icl_dbus_platform_info_to_gvariant(iotcon_platform_info_s *platform_info)
{
	GVariant *value;

	value = g_variant_new("(sssssssssss)",
			platform_info->platform_id,
			platform_info->manuf_name,
			platform_info->manuf_url,
			platform_info->model_number,
			platform_info->date_of_manufacture,
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

	return g_variant_new("a(qs)", builder);
}


GVariant* icl_dbus_observers_to_gvariant(iotcon_observers_h observers)
{
	GList *node;
	GVariantBuilder builder;

	g_variant_builder_init(&builder, G_VARIANT_TYPE("ai"));
	for (node = observers; node; node = node->next)
		g_variant_builder_add(&builder, "i", GPOINTER_TO_INT(node->data));

	return g_variant_builder_end(&builder);
}

