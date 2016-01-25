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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_REMOTE_RESOURCE_CACHING_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_REMOTE_RESOURCE_CACHING_H__

#include "iotcon-types.h"
#include "icl-types.h"

void icl_remote_resource_caching_table_insert(iotcon_remote_resource_h resource,
		icl_caching_container_s *cb_container);
int icl_remote_resource_caching_table_remove(iotcon_remote_resource_h resource);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_REMOTE_RESOURCE_CACHING_H__ */
