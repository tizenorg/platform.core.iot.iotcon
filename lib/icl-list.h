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
#ifndef __IOT_CONNECTIVITY_LIBRARY_LIST_H__
#define __IOT_CONNECTIVITY_LIBRARY_LIST_H__

#include <glib.h>

#include "iotcon-types.h"
#include "icl-value.h"

struct icl_list_s {
	int type;
	int ref_count;
	GList *list;
};

int icl_list_remove(iotcon_list_h list, iotcon_value_h val);
int icl_list_insert(iotcon_list_h list, iotcon_value_h value, int pos);

iotcon_list_h icl_list_ref(iotcon_list_h list);

#endif /* __IOT_CONNECTIVITY_LIBRARY_LIST_H__ */
