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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <tizen_type.h>

#include "iotcon-struct.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource-types.h"
#include "icl-query.h"

API int iotcon_query_create(iotcon_query_h *ret_query)
{
	RETV_IF(NULL == ret_query, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_query_h query = calloc(1, sizeof(struct icl_query));
	if (NULL == query) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	query->hash = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);

	*ret_query = query;

	return IOTCON_ERROR_NONE;
}


API void iotcon_query_destroy(iotcon_query_h query)
{
	RET_IF(NULL == query);

	g_hash_table_unref(query->hash);
	free(query);
}


API int iotcon_query_get_resource_types(iotcon_query_h query,
		iotcon_resource_types_h *types)
{
	int ret;
	char *resource_type = NULL;
	iotcon_resource_types_h types_temp = NULL;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_query_lookup(query, ICL_QUERY_KEY_RESOURCE_TYPE, &resource_type);
	if (NULL == resource_type) {
		ERR("resource_type is NULL");
		return IOTCON_ERROR_NO_DATA;
	}

	ret = iotcon_resource_types_create(&types_temp);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_create() Fail(%d)", ret);
		return ret;
	}

	ret = iotcon_resource_types_add(types_temp, resource_type);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_resource_types_add() Fail(%d)", ret);
		iotcon_resource_types_destroy(types_temp);
		return ret;
	}

	*types = types_temp;

	return IOTCON_ERROR_NONE;
}

API int iotcon_query_get_interface(iotcon_query_h query, int *iface)
{
	char *iface_str = NULL;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == iface, IOTCON_ERROR_INVALID_PARAMETER);

	iotcon_query_lookup(query, ICL_QUERY_KEY_INTERFACE, &iface_str);
	if (NULL == iface_str) {
		ERR("iface_str is NULL");
		return IOTCON_ERROR_NO_DATA;
	}

	*iface = atoi(iface_str);

	return IOTCON_ERROR_NONE;
}

API int iotcon_query_set_resource_types(iotcon_query_h query, iotcon_resource_types_h types)
{
	int length_old = 0;
	int length_new = 0;
	char *value = NULL;
	char *resource_type = NULL;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(query->hash, ICL_QUERY_KEY_RESOURCE_TYPE);
	if (value)
		length_old = (sizeof(ICL_QUERY_KEY_RESOURCE_TYPE) - 1) + strlen(value) + 2;

	if (types && types->type_list) {
		resource_type = types->type_list->data;
		length_new = (sizeof(ICL_QUERY_KEY_RESOURCE_TYPE) - 1) + strlen(resource_type) + 2;
	}

	if (ICL_QUERY_LENGTH_MAX < query->len - length_old + length_new) {
		ERR("Length of query is invalid.");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	if (value)
		iotcon_query_remove(query, ICL_QUERY_KEY_RESOURCE_TYPE);

	if (resource_type)
		iotcon_query_add(query, ICL_QUERY_KEY_RESOURCE_TYPE, resource_type);

	return IOTCON_ERROR_NONE;
}

API int iotcon_query_set_interface(iotcon_query_h query, iotcon_interface_e iface)
{
	int length_old = 0;
	int length_new = 0;
	char *value = NULL;
	char iface_str[ICL_QUERY_LENGTH_MAX] = {0};

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(query->hash, ICL_QUERY_KEY_INTERFACE);
	if (value)
		length_old = (sizeof(ICL_QUERY_KEY_INTERFACE) - 1) + strlen(value) + 2;

	snprintf(iface_str, sizeof(iface_str), "%d", iface);
	length_new = (sizeof(ICL_QUERY_KEY_INTERFACE) - 1) + strlen(iface_str) + 2;

	if (ICL_QUERY_LENGTH_MAX < query->len - length_old + length_new) {
		ERR("Length of query is invalid.");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	if (value)
		iotcon_query_remove(query, ICL_QUERY_KEY_INTERFACE);

	iotcon_query_add(query, ICL_QUERY_KEY_INTERFACE, iface_str);

	return IOTCON_ERROR_NONE;
}

/* The full length of query should be less than or equal to 64. */
API int iotcon_query_add(iotcon_query_h query, const char *key, const char *value)
{
	int query_len;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value, IOTCON_ERROR_INVALID_PARAMETER);

	/* first query : ?key=value
	 * Rest of query : &key=value */
	query_len = strlen(key) + strlen(value) + 2;
	if (ICL_QUERY_LENGTH_MAX < (query->len + query_len)) {
		ERR("Length of query is invalid.");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	g_hash_table_insert(query->hash, ic_utils_strdup(key), ic_utils_strdup(value));
	query->len += query_len;

	return IOTCON_ERROR_NONE;
}


API int iotcon_query_remove(iotcon_query_h query, const char *key)
{
	gboolean is_removed;
	int query_len;
	char *value;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(query->hash, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	query_len = strlen(key) + strlen(value) + 2;

	is_removed = g_hash_table_remove(query->hash, key);
	if (FALSE == is_removed) {
		ERR("g_hash_table_remove() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	query->len -= query_len;

	return IOTCON_ERROR_NONE;
}


API int iotcon_query_lookup(iotcon_query_h query, const char *key, char **data)
{
	char *value = NULL;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == data, IOTCON_ERROR_INVALID_PARAMETER);

	value = g_hash_table_lookup(query->hash, key);
	if (NULL == value) {
		ERR("g_hash_table_lookup() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*data = value;

	return IOTCON_ERROR_NONE;
}

API int iotcon_query_foreach(iotcon_query_h query, iotcon_query_foreach_cb cb,
		void *user_data)
{
	GHashTableIter iter;
	gpointer key, value;

	RETV_IF(NULL == query, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_iter_init(&iter, query->hash);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		if (IOTCON_FUNC_STOP == cb(key, value, user_data))
			break;
	}

	return IOTCON_ERROR_NONE;
}

