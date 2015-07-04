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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_COMMON_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_COMMON_H__

#include "iotcon-errors.h"

#ifndef IOTCON_DBUS_INTERFACE
#define IOTCON_DBUS_INTERFACE "org.tizen.iotcon.dbus"
#warning "IOTCON_DBUS_INTERFACE is redefined"
#endif

#define IOTCON_DBUS_OBJPATH "/org/tizen/iotcon/dbus"

#define IC_DBUS_SIGNAL_LENGTH 20

#define IC_DBUS_SIGNAL_REQUEST_HANDLER "REQ"
#define IC_DBUS_SIGNAL_FOUND_RESOURCE "RES"
#define IC_DBUS_SIGNAL_GET "GET"
#define IC_DBUS_SIGNAL_PUT "PUT"
#define IC_DBUS_SIGNAL_POST "POST"
#define IC_DBUS_SIGNAL_DELETE "DELETE"
#define IC_DBUS_SIGNAL_OBSERVE "OBSERVE"
#define IC_DBUS_SIGNAL_DEVICE "DEVICE"
#define IC_DBUS_SIGNAL_PLATFORM "PLATFORM"
#define IC_DBUS_SIGNAL_PRESENCE "PRESENCE"

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_COMMON_H__ */
