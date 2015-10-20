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
#include <errno.h>
#include <glib.h>

#include "iotcon.h"
#include "ic-dbus.h"
#include "icl.h"
#include "icl-dbus.h"
#include "icl-dbus-type.h"
#include "icl-remote-resource.h"

static void _caching_get_cb(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_options_h options,
		int response_result,
		void *user_data)
{
	int ret;
	iotcon_representation_h cloned_repr;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->caching_handle);
	RET_IF(IOTCON_RESPONSE_RESULT_OK != response_result);

	ret = iotcon_representation_clone(repr, &cloned_repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_clone() Fail(%d)", ret);
		return;
	}

	if (resource->caching_handle->repr)
		iotcon_representation_destroy(resource->caching_handle->repr);

	resource->caching_handle->repr = cloned_repr;

	if (resource->caching_handle->cb)
		resource->caching_handle->cb(resource, repr, resource->caching_handle->user_data);
}


static void _caching_observe_cb(GDBusConnection *connection,
		const gchar *sender_name,
		const gchar *object_path,
		const gchar *interface_name,
		const gchar *signal_name,
		GVariant *parameters,
		gpointer user_data)
{
	int ret;
	iotcon_remote_resource_h resource = user_data;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->caching_handle);

	ret = iotcon_remote_resource_get(resource, NULL, _caching_get_cb, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_remote_resource_get() for caching Fail(%d)", ret);
}


static void _caching_observe_cleanup(iotcon_remote_resource_h resource)
{
	resource->caching_handle->observe_handle = 0;
	resource->caching_handle->observe_sub_id = 0;
}


static int _caching_observer_start(iotcon_remote_resource_h resource)
{
	int ret;
	int64_t handle = 0;
	unsigned int sub_id = 0;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	ret = icl_remote_resource_observer_start(resource, IOTCON_OBSERVE, NULL,
			_caching_observe_cb, resource, _caching_observe_cleanup, &sub_id, &handle);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_remote_resource_observer_start() Fail(%d)", ret);
		return ret;
	}

	resource->caching_handle->observe_sub_id = sub_id;
	resource->caching_handle->observe_handle = handle;

	return IOTCON_ERROR_NONE;
}


static int _caching_observer_stop(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	if (0 == resource->caching_handle->observe_handle) {
		ERR("It doesn't have a caching observe handle");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ret = icl_remote_resource_observer_stop(resource,
			NULL,
			resource->caching_handle->observe_handle,
			resource->caching_handle->observe_sub_id);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_remote_resource_observer_stop() Fail(%d)", ret);
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


static gboolean _caching_get_timer(gpointer user_data)
{
	int ret;
	iotcon_remote_resource_h resource = user_data;

	RETV_IF(NULL == resource, G_SOURCE_REMOVE);

	ret = iotcon_remote_resource_get(resource, NULL, _caching_get_cb, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_remote_resource_get() for caching Fail(%d)", ret);

	return G_SOURCE_CONTINUE;
}


API int iotcon_remote_resource_start_caching(iotcon_remote_resource_h resource,
		int caching_interval,
		iotcon_remote_resource_cached_representation_changed_cb cb,
		void *user_data)
{
	int ret;
	unsigned int get_timer_id;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(ICL_REMOTE_RESOURCE_MAX_TIME_INTERVAL < caching_interval,
			IOTCON_ERROR_INVALID_PARAMETER);

	if (resource->caching_handle) {
		ERR("Already Start Caching");
		return IOTCON_ERROR_ALREADY;
	}

	INFO("Start Caching");

	resource->caching_handle = calloc(1, sizeof(struct icl_remote_resource_caching));
	if (NULL == resource->caching_handle) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	if (caching_interval <= 0) {
		WARN("Because time interval is negative, it sets default time interval.(10 sec)");
		resource->caching_handle->get_timer_interval
			= ICL_REMOTE_RESOURCE_DEFAULT_TIME_INTERVAL;
	} else {
		resource->caching_handle->get_timer_interval = caching_interval;
	}

	_caching_get_timer(resource);

	/* OBSERVE METHOD */
	ret = _caching_observer_start(resource);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_caching_observer_start() Fail(%d)", ret);
		free(resource->caching_handle);
		resource->caching_handle = NULL;
		return ret;
	}

	/* GET METHOD */
	resource->caching_handle->cb = cb;
	resource->caching_handle->user_data = user_data;

	get_timer_id = g_timeout_add_seconds(resource->caching_handle->get_timer_interval,
			_caching_get_timer, resource);
	resource->caching_handle->get_timer_id = get_timer_id;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_stop_caching(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == resource->caching_handle) {
		ERR("Not Cached");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	INFO("Stop Caching");

	/* GET METHOD */
	g_source_remove(resource->caching_handle->get_timer_id);

	if (resource->caching_handle->repr) {
		iotcon_representation_destroy(resource->caching_handle->repr);
		resource->caching_handle->repr = NULL;
	}

	/* OBSERVE METHOD */
	ret = _caching_observer_stop(resource);
	if (IOTCON_ERROR_NONE == ret) {
		ERR("_caching_observer_stop() Fail(%d)", ret);
		return ret;
	}

	free(resource->caching_handle);
	resource->caching_handle = NULL;

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_get_cached_representation(
		iotcon_remote_resource_h resource,
		iotcon_representation_h *representation)
{
	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == representation, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == resource->caching_handle->repr) {
		ERR("No Caching Representation");
		return IOTCON_ERROR_NO_DATA;
	}

	*representation = resource->caching_handle->repr;

	return IOTCON_ERROR_NONE;
}

