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
#include <glib.h>

#include "iotcon.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-struct.h"

API iotcon_query_h iotcon_query_new()
{
	return g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
}

API void iotcon_query_free(iotcon_query_h query)
{
	RET_IF(NULL == query);

	g_hash_table_unref(query);
}

API int iotcon_query_insert(iotcon_query_h query, const char *key, const char *value)
{
	RETV_IF(NULL == query, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == value, IOTCON_ERROR_PARAM);

	g_hash_table_insert(query, ic_utils_strdup(key), ic_utils_strdup(value));

	return IOTCON_ERROR_NONE;
}

API int iotcon_query_delete(iotcon_query_h query, const char *key)
{
	gboolean ret;

	RETV_IF(NULL == query, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == key, IOTCON_ERROR_PARAM);

	ret = g_hash_table_remove(query, key);
	if (FALSE == ret) {
		ERR("g_hash_table_remove() Fail");
		return IOTCON_ERROR_PARAM;
	}
	return IOTCON_ERROR_NONE;
}

API const char* iotcon_query_lookup(iotcon_query_h query, const char *key)
{
	const char *ret = NULL;

	RETV_IF(NULL == query, NULL);
	RETV_IF(NULL == key, NULL);

	ret = g_hash_table_lookup(query, key);
	if (NULL == ret)
		ERR("g_hash_table_lookup() Fail");

	return ret;
}

API void iotcon_query_foreach(iotcon_query_h query,
		iotcon_query_foreach_cb cb, void *user_data)
{
	GHashTableIter iter;
	gpointer key, value;

	RET_IF(NULL == query);
	RET_IF(NULL == cb);

	g_hash_table_iter_init(&iter, query);
	while (g_hash_table_iter_next(&iter, &key, &value))
		cb(key, value, user_data);
}

