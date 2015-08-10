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
#ifndef __IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_THREAD_H__
#define __IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_THREAD_H__

#include <glib.h>
#include <octypes.h>

void icd_ioty_ocprocess_stop();

gpointer icd_ioty_ocprocess_thread(gpointer data);

OCEntityHandlerResult icd_ioty_ocprocess_req_handler(OCEntityHandlerFlag flag,
		OCEntityHandlerRequest *request);

OCStackApplicationResult icd_ioty_ocprocess_find_cb(void *ctx, OCDoHandle handle,
		OCClientResponse* resp);

OCStackApplicationResult icd_ioty_ocprocess_get_cb(void *ctx, OCDoHandle handle,
		OCClientResponse* resp);

OCStackApplicationResult icd_ioty_ocprocess_put_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp);

OCStackApplicationResult icd_ioty_ocprocess_post_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp);

OCStackApplicationResult icd_ioty_ocprocess_delete_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp);

OCStackApplicationResult icd_ioty_ocprocess_observe_cb(void *ctx, OCDoHandle handle,
		OCClientResponse* resp);

OCStackApplicationResult icd_ioty_ocprocess_presence_cb(void *ctx, OCDoHandle handle,
		OCClientResponse* resp);

#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_IOTIVITY_THREAD_H__*/
