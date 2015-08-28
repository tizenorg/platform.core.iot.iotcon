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
#ifndef __IOT_CONNECTIVITY_MANAGER_DAEMON_DBUS_H__
#define __IOT_CONNECTIVITY_MANAGER_DAEMON_DBUS_H__

#include <stdint.h>
#include <glib.h>
#include "ic-dbus.h"

#define ICD_INT64_TO_POINTER(i) ((void*)(intptr_t)(i))
#define ICD_POINTER_TO_INT64(p) ((int64_t)(intptr_t)(p))

icDbus* icd_dbus_get_object();
int icd_dbus_client_list_get_info(void *handle, unsigned int *sig_num, gchar **bus_name);
int icd_dbus_emit_signal(const char *dest, const char *sig_name, GVariant *value);
unsigned int icd_dbus_init();
void icd_dbus_deinit(unsigned int id);

#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_DBUS_H__*/
