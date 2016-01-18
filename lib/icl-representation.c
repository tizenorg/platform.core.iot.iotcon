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
#include <limits.h>
#include <glib.h>

#include "iotcon-types.h"
#include "ic-utils.h"
#include "icl.h"
#include "icl-resource.h"
#include "icl-resource-types.h"
#include "icl-response.h"
#include "icl-list.h"
#include "icl-value.h"
#include "icl-state.h"
#include "icl-representation.h"

iotcon_representation_h icl_representation_ref(iotcon_representation_h repr)
{
	RETV_IF(NULL == repr, NULL);
	RETV_IF(repr->ref_count <= 0, NULL);

	repr->ref_count++;

	return repr;
}


API int iotcon_representation_create(iotcon_representation_h *ret_repr)
{
	errno = 0;
	iotcon_representation_h repr;

	RETV_IF(NULL == ret_repr, IOTCON_ERROR_INVALID_PARAMETER);

	repr = calloc(1, sizeof(struct icl_representation_s));
	if (NULL == repr) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	repr->visibility = (ICL_VISIBILITY_REPR | ICL_VISIBILITY_PROP);
	repr->ref_count = 1;

	*ret_repr = repr;

	return IOTCON_ERROR_NONE;
}


API void iotcon_representation_destroy(iotcon_representation_h repr)
{
	RET_IF(NULL == repr);

	repr->ref_count--;

	if (0 != repr->ref_count)
		return;

	free(repr->uri_path);

	/* (GDestroyNotify) : iotcon_representation_h is proper type than gpointer */
	g_list_free_full(repr->children, (GDestroyNotify)iotcon_representation_destroy);

	/* null COULD be allowed */
	if (repr->res_types)
		iotcon_resource_types_destroy(repr->res_types);

	/* null COULD be allowed */
	if (repr->state)
		iotcon_state_destroy(repr->state);

	free(repr);
}


API int iotcon_representation_get_uri_path(iotcon_representation_h repr, char **uri_path)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	*uri_path = repr->uri_path;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_uri_path(iotcon_representation_h repr,
		const char *uri_path)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == uri_path, IOTCON_ERROR_INVALID_PARAMETER);

	free(repr->uri_path);
	repr->uri_path = strdup(uri_path);
	if (NULL == repr->uri_path) {
		ERR("strdup() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_resource_types(iotcon_representation_h repr,
		iotcon_resource_types_h *types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == types, IOTCON_ERROR_INVALID_PARAMETER);

	*types = repr->res_types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_resource_types(iotcon_representation_h repr,
		iotcon_resource_types_h types)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	if (types)
		types = icl_resource_types_ref(types);

	if (repr->res_types)
		iotcon_resource_types_destroy(repr->res_types);

	repr->res_types = types;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_resource_interfaces(iotcon_representation_h repr,
		int *ifaces)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == ifaces, IOTCON_ERROR_INVALID_PARAMETER);

	*ifaces = repr->interfaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_resource_interfaces(iotcon_representation_h repr,
		int ifaces)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	RETV_IF(ifaces <= IOTCON_INTERFACE_NONE || IC_INTERFACE_MAX < ifaces,
			IOTCON_ERROR_INVALID_PARAMETER);

	repr->interfaces = ifaces;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_set_state(iotcon_representation_h repr,
		iotcon_state_h state)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);

	if (state)
		state = icl_state_ref(state);

	if (repr->state)
		iotcon_state_destroy(repr->state);

	repr->state = state;

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_get_state(iotcon_representation_h repr,
		iotcon_state_h *state)
{
	RETV_IF(NULL == repr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	*state = repr->state;

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_add_child(iotcon_representation_h parent,
		iotcon_representation_h child)
{
	iotcon_representation_h repr;

	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	repr = icl_representation_ref(child);

	parent->children = g_list_append(parent->children, repr);

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_remove_child(iotcon_representation_h parent,
		iotcon_representation_h child)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == child, IOTCON_ERROR_INVALID_PARAMETER);

	parent->children = g_list_remove(parent->children, child);

	iotcon_representation_destroy(child);

	return IOTCON_ERROR_NONE;
}


API int iotcon_representation_foreach_children(iotcon_representation_h parent,
		iotcon_children_cb cb, void *user_data)
{
	GList *list, *next;

	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	list = parent->children;
	while (list) {
		next = list->next;
		if (IOTCON_FUNC_STOP == cb(list->data, user_data))
			break;
		list = next;
	}

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_children_count(iotcon_representation_h parent,
		unsigned int *count)
{
	RETV_IF(NULL == parent, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == count, IOTCON_ERROR_INVALID_PARAMETER);
	if (NULL == parent->children)
		*count = 0;
	else
		*count = g_list_length(parent->children);

	return IOTCON_ERROR_NONE;
}

API int iotcon_representation_get_nth_child(iotcon_representation_h parent, int pos,
		iotcon_representation_h *child)
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


API int iotcon_representation_clone(const iotcon_representation_h src,
		iotcon_representation_h *dest)
{
	FN_CALL;
	int ret;
	GList *node;
	iotcon_resource_types_h list;
	iotcon_representation_h cloned_repr, copied_repr;

	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_representation_create(&cloned_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return ret;
	}

	if (src->uri_path) {
		cloned_repr->uri_path = strdup(src->uri_path);
		if (NULL == cloned_repr->uri_path) {
			ERR("strdup() Fail");
			iotcon_representation_destroy(cloned_repr);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
	}

	if (src->interfaces)
		cloned_repr->interfaces = src->interfaces;

	if (src->res_types) {
		ret = iotcon_resource_types_clone(src->res_types, &list);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_types_clone() Fail");
			iotcon_representation_destroy(cloned_repr);
			return ret;
		}
		cloned_repr->res_types = list;
	}

	if (src->children) {
		for (node = g_list_first(src->children); node; node = node->next) {
			ret = iotcon_representation_clone((iotcon_representation_h)node->data,
					&copied_repr);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_representation_clone(child) Fail(%d)", ret);
				iotcon_representation_destroy(cloned_repr);
				return ret;
			}
			cloned_repr->children = g_list_append(cloned_repr->children, copied_repr);
		}
	}

	if (src->state) {
		ret = iotcon_state_clone(src->state, &cloned_repr->state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_state_clone() Fail(%d)", ret);
			iotcon_representation_destroy(cloned_repr);
			return ret;
		}
	}

	*dest = cloned_repr;

	return IOTCON_ERROR_NONE;
}
