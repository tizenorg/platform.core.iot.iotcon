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

#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource-types.h"

iotcon_resource_types_h icl_resource_types_ref(iotcon_resource_types_h types)
{
	RETV_IF(NULL == types, NULL);
	RETV_IF(types->ref_count <= 0, NULL);

	types->ref_count++;

	return types;
}


API iotcon_resource_types_h iotcon_resource_types_new()
{
	iotcon_resource_types_h types = calloc(1, sizeof(struct icl_resource_types));
	if (NULL == types) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	types->ref_count = 1;

	return types;
}


API void iotcon_resource_types_free(iotcon_resource_types_h types)
{
	RET_IF(NULL == types);

	types->ref_count--;

	if (0 == types->ref_count) {
		g_list_free_full(types->type_list, free);
		free(types);
	}
}


static int _icl_resource_types_strcmp(const void *a, const void *b)
{
	return strcmp(a, b);
}


static bool _icl_resource_types_duplicate_check(iotcon_resource_types_h types,
		const char *type)
{
	GList *ret = NULL;

	RETV_IF(NULL == types, false);
	RETV_IF(NULL == type, false);

	ret = g_list_find_custom(types->type_list, type, _icl_resource_types_strcmp);
	if (NULL == ret)
		return false;

	return true;
}


/* The length of resource type should be less than or equal to 61.
 * Duplicate strings are not allowed. */
API int iotcon_resource_types_insert(iotcon_resource_types_h types, const char *type)
{
	char *resource_type;

	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == type, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(1 < types->ref_count, IOTCON_ERROR_INVALID_PARAMETER,
			"Don't modify it. It is already set.");

	if (IOTCON_RESOURCE_TYPE_LENGTH_MAX < strlen(type)) {
		ERR("The length of type(%s) should be less than or equal to %d.", type,
				IOTCON_RESOURCE_TYPE_LENGTH_MAX);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	if (true == _icl_resource_types_duplicate_check(types, type)) {
		ERR("%s is already contained.", type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	resource_type = strdup(type);
	if (NULL == resource_type) {
		ERR("strdup() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	types->type_list = g_list_append(types->type_list, resource_type);

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_types_delete(iotcon_resource_types_h types, const char *type)
{
	GList *found_node;

	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == type, IOTCON_ERROR_INVALID_PARAMETER);
	RETVM_IF(1 < types->ref_count, IOTCON_ERROR_INVALID_PARAMETER,
			"Don't modify it. It is already set.");

	found_node = g_list_find_custom(types->type_list, type, _icl_resource_types_strcmp);
	if (NULL == found_node) {
		ERR("g_list_find_custom() Fail");
		return IOTCON_ERROR_NO_DATA;
	}

	free(found_node->data);
	types->type_list = g_list_delete_link(types->type_list, found_node);

	return IOTCON_ERROR_NONE;
}


API int iotcon_resource_types_foreach(iotcon_resource_types_h types,
		iotcon_resource_types_foreach_cb cb, void *user_data)
{
	GList *node;

	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	for (node = types->type_list; node; node = node->next) {
		if (IOTCON_FUNC_STOP == cb((const char*)node->data, user_data))
			break;
	}

	return IOTCON_ERROR_NONE;
}


API iotcon_resource_types_h iotcon_resource_types_clone(iotcon_resource_types_h types)
{
	GList *node;
	char *resource_type;
	iotcon_resource_types_h clone;

	RETV_IF(NULL == types, NULL);

	clone = calloc(1, sizeof(struct icl_resource_types));
	if (NULL == clone) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	for (node = types->type_list; node; node = node->next) {
		resource_type = ic_utils_strdup(node->data);
		if (NULL == resource_type) {
			iotcon_resource_types_free(clone);
			ERR("ic_utils_strdup() Fail");
			return NULL;
		}
		clone->type_list = g_list_append(clone->type_list, resource_type);
	}

	clone->ref_count = 1;

	return clone;
}


/* counting from 0 */
const char* icl_resource_types_get_nth_data(iotcon_resource_types_h types, int index)
{
	return g_list_nth_data(types->type_list, index);
}


unsigned int icl_resource_types_get_length(iotcon_resource_types_h types)
{
	return g_list_length(types->type_list);
}
