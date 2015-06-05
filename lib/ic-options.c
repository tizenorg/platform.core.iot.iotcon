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
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <glib.h>

#include "iotcon-struct.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-options.h"

API iotcon_options_h iotcon_options_new()
{
	iotcon_options_h options = calloc(1, sizeof(struct ic_options));
	if (NULL == options) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	options->hash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, free);
	return options;
}


void ic_options_free(iotcon_options_h options)
{
	RET_IF(NULL == options);

	g_hash_table_unref(options->hash);
	free(options);
}


API void iotcon_options_free(iotcon_options_h options)
{
	RET_IF(NULL == options);
	RETM_IF(true == options->has_parent, "iotcon_options has parent");

	ic_options_free(options);
}


/* iotcon_options_h can have up to 2 options.
 * option id is always situated between 2048 and 3000.
 * Length of option data is less than or equal to 15. */
API int iotcon_options_insert(iotcon_options_h options, unsigned short id,
		const char *data)
{
	FN_CALL;

	RETV_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(options->has_parent, IOTCON_ERROR_INVALID_PARAMETER,
			"Don't modify it. It is already set.");
	RETVM_IF(IOTCON_OPTIONS_MAX <= g_hash_table_size(options->hash),
			IOTCON_ERROR_OUT_OF_MEMORY, "Options already have maximum elements.");

	RETVM_IF(((id < IOTCON_OPTIONID_MIN) || (IOTCON_OPTIONID_MAX < id)),
			IOTCON_ERROR_INVALID_PARAMETER, "Invalid id(%d)", id);

	RETV_IF(NULL == data, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(IOTCON_OPTION_DATA_LENGTH_MAX < strlen(data), IOTCON_ERROR_INVALID_PARAMETER,
			"The length of option data(%s) is invalid.", data);

	g_hash_table_insert(options->hash, GUINT_TO_POINTER(id), ic_utils_strdup(data));

	return IOTCON_ERROR_NONE;
}


API int iotcon_options_delete(iotcon_options_h options, unsigned short id)
{
	gboolean ret;

	RETV_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(options->has_parent, IOTCON_ERROR_INVALID_PARAMETER,
			"Don't modify it. It is already set.");

	ret = g_hash_table_remove(options->hash, GUINT_TO_POINTER(id));
	if (FALSE == ret) {
		ERR("g_hash_table_remove() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


API const char* iotcon_options_lookup(iotcon_options_h options, unsigned short id)
{
	const char *ret;

	RETV_IF(NULL == options, NULL);

	ret = g_hash_table_lookup(options->hash, GUINT_TO_POINTER(id));
	if (NULL == ret)
		ERR("g_hash_table_lookup() Fail");

	return ret;
}


API int iotcon_options_foreach(iotcon_options_h options,
		iotcon_options_foreach_cb cb, void *user_data)
{
	GHashTableIter iter;
	gpointer key, value;

	RETV_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_iter_init(&iter, options->hash);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		if (false == cb(GPOINTER_TO_UINT(key), value, user_data))
			break;
	}

	return IOTCON_ERROR_NONE;
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

	ref->hash = g_hash_table_ref(options->hash);

	return ref;
}

