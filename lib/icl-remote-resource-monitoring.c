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
#include "icl.h"
#include "icl-remote-resource.h"

static void _monitoring_get_cb(iotcon_remote_resource_h resource,
		iotcon_representation_h repr,
		iotcon_options_h options,
		int response_result,
		void *user_data)
{
	iotcon_remote_resource_state_e resource_state;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->monitoring_handle);

	if (IOTCON_RESPONSE_RESULT_OK <= response_result)
		resource_state = IOTCON_REMOTE_RESOURCE_STATE_ALIVE;
	else
		resource_state = IOTCON_REMOTE_RESOURCE_STATE_LOST_SIGNAL;

	if (resource_state == resource->monitoring_handle->resource_state)
		return;

	resource->monitoring_handle->resource_state = resource_state;

	if (resource->monitoring_handle->cb)
		resource->monitoring_handle->cb(resource, resource_state,
				resource->monitoring_handle->user_data);
}


static gboolean _monitoring_get_timer(gpointer user_data)
{
	int ret;
	iotcon_remote_resource_h resource = user_data;

	RETV_IF(NULL == resource, G_SOURCE_REMOVE);

	ret = iotcon_remote_resource_get(resource, NULL, _monitoring_get_cb, NULL);
	if (IOTCON_ERROR_NONE != ret)
		ERR("iotcon_remote_resource_get() for caching Fail(%d)", ret);

	return G_SOURCE_CONTINUE;
}


static void _monitoring_presence_cb(int result, unsigned int nonce,
		const char *host_address, void *user_data)
{
	unsigned int get_timer_id;
	iotcon_remote_resource_h resource = user_data;

	RET_IF(NULL == resource);
	RET_IF(NULL == resource->monitoring_handle);

	g_source_remove(resource->monitoring_handle->get_timer_id);

	_monitoring_get_timer(resource);
	get_timer_id = g_timeout_add_seconds(resource->caching_handle->get_timer_interval,
			_monitoring_get_timer, resource);
	resource->monitoring_handle->get_timer_id = get_timer_id;
}


API int iotcon_remote_resource_start_monitoring(iotcon_remote_resource_h resource,
		int monitoring_interval,
		iotcon_remote_resource_state_changed_cb cb,
		void *user_data)
{
	int ret;
	char *host_address;
	unsigned int get_timer_id;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(ICL_REMOTE_RESOURCE_MAX_TIME_INTERVAL < monitoring_interval,
			IOTCON_ERROR_INVALID_PARAMETER);

	if (resource->monitoring_handle) {
		ERR("Already Start Monitoring");
		return IOTCON_ERROR_ALREADY;
	}

	INFO("Start Monitoring");

	resource->monitoring_handle = calloc(1, sizeof(struct icl_remote_resource_monitoring));
	if (NULL == resource->monitoring_handle) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	if (monitoring_interval <= 0) {
		WARN("Because time interval is negative, it sets default time interval.(10 sec)");
		resource->monitoring_handle->get_timer_interval
			= ICL_REMOTE_RESOURCE_DEFAULT_TIME_INTERVAL;
	} else {
		resource->monitoring_handle->get_timer_interval = monitoring_interval;
	}

	_monitoring_get_timer(resource);

	/* GET METHOD (Resource Presence) */
	resource->monitoring_handle->cb = cb;
	resource->monitoring_handle->user_data = user_data;

	get_timer_id = g_timeout_add_seconds(resource->monitoring_handle->get_timer_interval,
			_monitoring_get_timer, resource);
	resource->monitoring_handle->get_timer_id = get_timer_id;

	/* Device Presence */
	ret = iotcon_remote_resource_get_host(resource, &host_address);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_remote_resource_get_host() Fail(%d)", ret);
		g_source_remove(resource->monitoring_handle->get_timer_id);
		free(resource->monitoring_handle);
		resource->monitoring_handle = NULL;
		return ret;
	}

	ret = iotcon_subscribe_presence(host_address, NULL, _monitoring_presence_cb, resource,
			&resource->monitoring_handle->presence);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_subscribe_presence() Fail(%d)", ret);
		g_source_remove(resource->monitoring_handle->get_timer_id);
		free(resource->monitoring_handle);
		resource->monitoring_handle = NULL;
		return ret;
	}

	return IOTCON_ERROR_NONE;
}


API int iotcon_remote_resource_stop_monitoring(iotcon_remote_resource_h resource)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == resource->monitoring_handle) {
		ERR("Not Monitoring");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	INFO("Stop Monitoring");

	/* Device Presence */
	ret = iotcon_unsubscribe_presence(resource->monitoring_handle->presence);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_unsubscribe_presence() Fail(%d)", ret);
		return ret;
	}

	/* GET METHOD */
	g_source_remove(resource->monitoring_handle->get_timer_id);

	free(resource->monitoring_handle);
	resource->monitoring_handle = NULL;

	return IOTCON_ERROR_NONE;
}
