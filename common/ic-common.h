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

#define IC_INT64_TO_POINTER(i) ((void*)(intptr_t)(i))
#define IC_POINTER_TO_INT64(p) ((int64_t)(intptr_t)(p))

#define IC_REMOTE_RESOURCE_DEFAULT_TIME_INTERVAL 10 /* 10 sec */
#define IC_PRESENCE_TTL_SECONDS_DEFAULT 60 /* 60 sec */
#define IC_PRESENCE_TTL_SECONDS_MAX (60 * 60 * 24) /* 60 sec/min * 60 min/hr * 24 hr/day */

#define IOTCON_DBUS_OBJPATH "/org/tizen/iotcon/dbus"
#define IC_OBSERVE_ID_MAX_LEN 256


#define IC_DBUS_SIGNAL_LENGTH 30

#define IC_DBUS_SIGNAL_REQUEST_HANDLER "REQ"
#define IC_DBUS_SIGNAL_FOUND_RESOURCE "RES"
#define IC_DBUS_SIGNAL_OBSERVE "OBSERVE"
#define IC_DBUS_SIGNAL_DEVICE "DEVICE"
#define IC_DBUS_SIGNAL_PLATFORM "PLATFORM"
#define IC_DBUS_SIGNAL_PRESENCE "PRESENCE"
#define IC_DBUS_SIGNAL_MONITORING "MONITORING"
#define IC_DBUS_SIGNAL_CACHING "CACHING"

#define IC_FEATURE_OIC "http://tizen.org/feature/iot.oic"

#define IC_IOTY_COAP "coap://"
#define IC_IOTY_COAPS "coaps://"
#define IC_IOTY_MULTICAST_ADDRESS "224.0.1.187:5683"

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_COMMON_H__ */
