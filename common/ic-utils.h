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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_COMMON_UTILITY_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_COMMON_UTILITY_H__

#include <glib.h>

#include "iotcon-types.h"

#define IC_EQUAL 0
#define IC_STR_EQUAL 0
#define IC_STR_NULL "(NULL)"

#define IC_INTERFACE_MAX (IOTCON_INTERFACE_DEFAULT | IOTCON_INTERFACE_LINK | \
		IOTCON_INTERFACE_BATCH | IOTCON_INTERFACE_GROUP)

char* ic_utils_strdup(const char *src);
const char* ic_utils_dbus_encode_str(const char *src);
char* ic_utils_dbus_decode_str(char *src);
int ic_utils_convert_interface_flag(iotcon_interface_e src, char **dest);
int ic_utils_convert_interface_string(const char *src, iotcon_interface_e *dest);
void ic_utils_gvariant_array_free(GVariant **value);

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_COMMON_UTILITY_H__ */
