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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_INTERFACES_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_INTERFACES_H__

#include <glib.h>
#include "iotcon-types.h"

/**
 * @brief The maximum length which can be held in a resource interface.
 *
 * @since_tizen 3.0
 */

struct icl_resource_ifaces {
	int ref_count;
	GList *iface_list;
};

iotcon_resource_ifaces_h icl_resource_ifaces_ref(iotcon_resource_ifaces_h res_ifaces);
const char* icl_resource_ifaces_get_nth_data(iotcon_resource_ifaces_h res_ifaces, int index);
unsigned int icl_resource_ifaces_get_length(iotcon_resource_ifaces_h res_ifaces);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_RESOURCE_INTERFACES_H__ */
