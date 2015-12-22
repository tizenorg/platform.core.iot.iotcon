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
#ifndef __IOT_CONNECTIVITY_MANAGER_DAEMON_CYNARA_H__
#define __IOT_CONNECTIVITY_MANAGER_DAEMON_CYNARA_H__

#include <gio/gio.h>

int icd_cynara_init();
void icd_cynara_deinit();
int icd_cynara_check_network(GDBusMethodInvocation *invocation);
int icd_cynara_check_data(GDBusMethodInvocation *invocation);
int icd_cynara_check_device(GDBusMethodInvocation *invocation);

#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_CYNARA_H__*/
