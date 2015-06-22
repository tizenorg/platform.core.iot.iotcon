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

int icd_dbus_bus_list_get_info(int handle, unsigned int *sig_num, const gchar **sender);
int icd_dbus_emit_signal(const char *sig_name, const char *sender, GVariant *value);
unsigned int icd_dbus_init();
void icd_dbus_deinit(unsigned int id);


#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_DBUS_H__*/
