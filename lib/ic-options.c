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
#include "ic-options.h"

API iotcon_options_h iotcon_options_new()
{
	iotcon_options_h options = calloc(1, sizeof(struct ic_options));
	if (NULL == options) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	options->options = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, free);
	return options;
}


void ic_options_free(iotcon_options_h options)
{
	RET_IF(NULL == options);

	g_hash_table_unref(options->options);
	free(options);
}


API void iotcon_options_free(iotcon_options_h options)
{
	RET_IF(NULL == options);
	RETM_IF(true == options->has_parent, "iotcon_options has parent");

	ic_options_free(options);
}


/* options id is always situated between 2014 and 3000 */
API int iotcon_options_insert(iotcon_options_h options, unsigned short id,
		const char *data)
{
	FN_CALL;

	RETV_IF(NULL == options, IOTCON_ERROR_PARAM);
	RETVM_IF(((id < IOTCON_OPTIONID_MIN) || (IOTCON_OPTIONID_MAX < id)),
			IOTCON_ERROR_PARAM, "Invalid id(%d)", id);
	RETV_IF(NULL == data, IOTCON_ERROR_PARAM);

	g_hash_table_insert(options->options, GUINT_TO_POINTER(id), ic_utils_strdup(data));

	return IOTCON_ERROR_NONE;
}


API int iotcon_options_delete(iotcon_options_h options, unsigned short id)
{
	gboolean ret;

	RETV_IF(NULL == options, IOTCON_ERROR_PARAM);

	ret = g_hash_table_remove(options->options, GUINT_TO_POINTER(id));
	if (FALSE == ret) {
		ERR("g_hash_table_remove() Fail");
		return IOTCON_ERROR_PARAM;
	}
	return IOTCON_ERROR_NONE;
}


API const char* iotcon_options_lookup(iotcon_options_h options, unsigned short id)
{
	const char *ret;

	RETV_IF(NULL == options, NULL);

	ret = g_hash_table_lookup(options->options, GUINT_TO_POINTER(id));
	if (NULL == ret)
		ERR("g_hash_table_lookup() Fail");

	return ret;
}


API void iotcon_options_foreach(iotcon_options_h options,
		iotcon_options_foreach_cb cb, void *user_data)
{
	GHashTableIter iter;
	gpointer key, value;

	RET_IF(NULL == options);
	RET_IF(NULL == cb);

	g_hash_table_iter_init(&iter, options->options);
	while (g_hash_table_iter_next(&iter, &key, &value))
		cb(GPOINTER_TO_UINT(key), value, user_data);
}


iotcon_options_h ic_options_ref(iotcon_options_h options)
{
	iotcon_options_h ref;

	RETV_IF(NULL == options, NULL);

	ref = calloc(1, sizeof(struct ic_options));
	if (NULL == ref) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	ref->options = g_hash_table_ref(options->options);

	return ref;
}

