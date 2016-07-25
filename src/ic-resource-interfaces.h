/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#ifndef __IOTCON_INTERNAL_RESOURCE_INTERFACES_H__
#define __IOTCON_INTERNAL_RESOURCE_INTERFACES_H__

#include <glib.h>
#include "iotcon-types.h"

struct icl_resource_ifaces {
	int ref_count;
	GList *iface_list;
};

iotcon_resource_interfaces_h icl_resource_interfaces_ref(
		iotcon_resource_interfaces_h res_ifaces);

#endif /* __IOTCON_INTERNAL_RESOURCE_INTERFACES_H__ */
