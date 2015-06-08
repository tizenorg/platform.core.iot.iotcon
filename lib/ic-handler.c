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

#include "iotcon.h"

#include "ic-common.h"
#include "ic-ioty.h"
#include "ic-handler.h"

static gboolean _find_valid_resource(gpointer key, gpointer value, gpointer user_data)
{
	return (key == user_data);
}

resource_handler_s* ic_get_resource_handler_data(void *handle)
{
	resource_handler_s *handler;
	ic_ctx_s *ic_ctx = ic_get_ctx();

	handler = g_hash_table_find(ic_ctx->entity_cb_hash, _find_valid_resource, handle);
	return handler;
}

void ic_get_device_info_handler(iotcon_device_info_s *info)
{
	FN_CALL;
	ic_ctx_s *ic_ctx = ic_get_ctx();
	iotcon_found_device_info_cb notify_cb = NULL;

	GList *node = g_list_first(ic_ctx->found_device_cb_lst);
	while (node) {
		notify_cb = node->data;
		if (notify_cb)
			notify_cb(info);

		node = node->next;
	}
}
