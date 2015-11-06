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
#ifndef __IOT_CONNECTIVITY_MANAGER_DBUS_H__
#define __IOT_CONNECTIVITY_MANAGER_DBUS_H__

#include "ic-dbus.h"

#define ICL_DBUS_TIMEOUT_DEFAULT 30 /* 30 sec */
#define ICL_DBUS_TIMEOUT_MAX 60 /* 60 sec */


icDbus* icl_dbus_get_object();
unsigned int icl_dbus_generate_signal_number();

unsigned int icl_dbus_subscribe_signal(char *signal_name, void *cb_container,
		void *cb_free, GDBusSignalCallback sig_handler);
void icl_dbus_unsubscribe_signal(unsigned int id);

int icl_dbus_add_connection_changed_cb(iotcon_connection_changed_cb cb,
		void *user_data);
int icl_dbus_remove_connection_changed_cb(iotcon_connection_changed_cb cb,
		void *user_data);

int icl_dbus_convert_daemon_error(int error);
int icl_dbus_convert_dbus_error(int error);

int icl_dbus_set_timeout(int timeout_seconds);
int icl_dbus_get_timeout();

int icl_dbus_start();
void icl_dbus_stop();

#endif /* __IOT_CONNECTIVITY_MANAGER_DBUS_H__ */
