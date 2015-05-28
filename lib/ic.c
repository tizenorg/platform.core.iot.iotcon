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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <glib-object.h>

#include "iotcon.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-ioty.h"
#include "ic.h"

/**
 * @brief global context
 */
static GHashTable *ic_request_cb_hash;
static bool ic_is_init = false;

static void _free_resource(gpointer data)
{
	int ret;
	iotcon_resource_h resource = data;

	RET_IF(NULL == data);

	ret = ic_ioty_unregister_res(resource->handle);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_unregister_res() Fail(%d)", ret);

	free(resource);
}

/* Host address should begin with "coap://" */
API void iotcon_initialize(const char *addr, unsigned short port)
{
	FN_CALL;

	RETM_IF(true == ic_is_init, "already initialized");
	RET_IF(NULL == addr);

	ic_ioty_config(addr, port);
	ic_request_cb_hash = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
			_free_resource);

#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif
	ic_is_init = true;
}


API void iotcon_deinitialize()
{
	FN_CALL;

	RETM_IF(false == ic_is_init, "Not initialized");

	g_hash_table_destroy(ic_request_cb_hash);
	ic_request_cb_hash = NULL;

	ic_is_init = false;
}


static gboolean _find_valid_resource(gpointer key, gpointer value, gpointer user_data)
{
	return (key == user_data);
}


ic_resource_s* ic_get_resource_handler_data(void *handle)
{
	return g_hash_table_find(ic_request_cb_hash, _find_valid_resource, handle);
}


/* The length of uri should be less than and equal to 36. */
API iotcon_resource_h iotcon_register_resource(const char *uri,
		iotcon_str_list_s *res_types,
		int ifaces,
		uint8_t properties,
		iotcon_request_handler_cb cb,
		void *user_data)
{
	FN_CALL;
	int i;
	iotcon_resource_h resource;

	RETV_IF(NULL == uri, NULL);
	RETVM_IF(IOTCON_URI_LENGTH_MAX < strlen(uri), NULL,
			"The length of uri(%s) is invalid", uri);
	RETV_IF(NULL == res_types, NULL);
	RETV_IF(NULL == cb, NULL);

	for (i = 0; i < iotcon_str_list_length(res_types); i++) {
		if (IOTCON_RESOURCE_TYPE_LENGTH_MAX
				< strlen(iotcon_str_list_nth_data(res_types, i))) {
			ERR("The length of resource_type is invalid");
			return NULL;
		}
	}

	resource = calloc(1, sizeof(struct _resource_s));
	if (NULL == resource) {
		ERR("calloc Fail(%d)", errno);
		return NULL;
	}

	resource->handle = ic_ioty_register_res(uri, res_types, ifaces, properties);
	if (NULL == resource->handle) {
		ERR("ic_ioty_register_res() Fail");
		free(resource);
		return NULL;
	}

	resource->cb = cb;
	resource->user_data = user_data;

	g_hash_table_insert(ic_request_cb_hash, resource->handle, resource);

	return resource;
}


API void iotcon_unregister_resource(iotcon_resource_h resource)
{
	FN_CALL;

	RET_IF(NULL == resource);

	g_hash_table_remove(ic_request_cb_hash, resource->handle);
}


API int iotcon_bind_interface(iotcon_resource_h resource, iotcon_interface_e iface)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_PARAM);

	ret = ic_ioty_bind_iface_to_res(resource->handle, iface);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_bind_iface_to_res() Fail(%d)", ret);

	return ret;
}


API int iotcon_bind_type(iotcon_resource_h resource, const char *resource_type)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == resource_type, IOTCON_ERROR_PARAM);
	if (IOTCON_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type)) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return IOTCON_ERROR_PARAM;
	}

	ret = ic_ioty_bind_type_to_res(resource->handle, resource_type);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_bind_type_to_res() Fail(%d)", ret);

	return ret;
}


API int iotcon_bind_resource(iotcon_resource_h parent, iotcon_resource_h child)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == parent, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == child, IOTCON_ERROR_PARAM);

	ret = ic_ioty_bind_res(parent->handle, child->handle);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_bind_res() Fail(%d)", ret);

	return ret;
}


API int iotcon_start_presence(unsigned int time_to_live)
{
	FN_CALL;
	int ret;

	ret = ic_ioty_start_presence(time_to_live);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_start_presence() Fail(%d)", ret);

	return ret;
}


API int iotcon_stop_presence()
{
	FN_CALL;
	int ret;

	ret = ic_ioty_stop_presence();
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_stop_presence() Fail(%d)", ret);

	return ret;
}


/* The length of resource_type should be less than and equal to 61. */
API iotcon_presence_h iotcon_subscribe_presence(const char *host_address,
		const char *resource_type, iotcon_presence_cb cb, void *user_data)
{
	iotcon_presence_h handle;

	RETV_IF(NULL == host_address, NULL);
	RETV_IF(NULL == cb, NULL);
	if (resource_type &&(IOTCON_RESOURCE_TYPE_LENGTH_MAX < strlen(resource_type))) {
		ERR("The length of resource_type(%s) is invalid", resource_type);
		return NULL;
	}

	if (NULL == resource_type)
		resource_type = "";

	handle = ic_ioty_subscribe_presence(host_address, resource_type, cb, user_data);
	if (NULL == handle)
		ERR("ic_ioty_subscribe_presence() Fail");

	return handle;
}


API int iotcon_unsubscribe_presence(iotcon_presence_h handle)
{
	FN_CALL;
	int ret;

	RETV_IF(NULL == handle, IOTCON_ERROR_PARAM);

	ret = ic_ioty_unsubscribe_presence(handle);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_unsubscribe_presence() Fail(%d)", ret);

	return ret;
}


API iotcon_notimsg_h iotcon_notimsg_new(iotcon_repr_h repr, iotcon_interface_e iface)
{
	iotcon_notimsg_h msg;

	RETV_IF(NULL == repr, NULL);

	msg = calloc(1, sizeof(struct ic_notify_msg));
	if (NULL == msg) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	msg->repr = repr;
	msg->iface = iface;
	msg->error_code = 200;

	return msg;
}


API void iotcon_notimsg_free(iotcon_notimsg_h msg)
{
	RET_IF(NULL == msg);

	iotcon_repr_free(msg->repr);
	free(msg);
}


API int iotcon_notify(iotcon_resource_h resource, iotcon_notimsg_h msg,
		iotcon_observers_h observers)
{
	int ret;

	RETV_IF(NULL == resource, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == observers, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == msg, IOTCON_ERROR_PARAM);
	RETV_IF(NULL == msg->repr, IOTCON_ERROR_PARAM);

	ret = ic_ioty_send_notify(resource->handle, msg, observers);
	if (IOTCON_ERROR_NONE != ret)
		ERR("ic_ioty_send_notify() Fail(%d)", ret);

	return ret;
}
