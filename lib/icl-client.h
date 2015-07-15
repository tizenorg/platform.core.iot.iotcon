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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_CLIENT_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_CLIENT_H__

#include "iotcon-struct.h"
#include "icl-options.h"
#include <json-glib/json-glib.h>

struct icl_remote_resource {
	int ref_count;
	char *uri_path;
	char *host;
	char *sid;
	bool is_observable;
	bool is_collection;
	iotcon_options_h header_options;
	iotcon_resource_types_h types;
	int ifaces;
	iotcon_connectivity_type_e conn_type;
	int observe_handle;
	unsigned int observe_sub_id;
};

iotcon_client_h icl_client_parse_resource_object(JsonParser *parser, char *json_string,
		const char *host, iotcon_connectivity_type_e conn_type);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_CLIENT_H__ */
