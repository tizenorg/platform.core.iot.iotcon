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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_H__

#include "iotcon.h"
#include "icl-dbus.h"

struct icl_notify_msg {
	int error_code;
	iotcon_interface_e iface;
	iotcon_repr_h repr;
};

struct icl_resource {
	char *uri;
	char *host;
	bool is_observable;
	iotcon_resource_types_h types;
	int ifaces;
	icl_handle_container_s *handle;
	iotcon_request_handler_cb cb;
	void *user_data;
	iotcon_resource_h children[IOTCON_CONTAINED_RESOURCES_MAX];
};

int icl_ioty_convert_interface_flag(iotcon_interface_e src, char **dest);
int icl_ioty_convert_interface_string(const char *src, iotcon_interface_e *dest);


#endif /*__IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_H__*/
