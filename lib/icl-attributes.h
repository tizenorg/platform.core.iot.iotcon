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
#ifndef __IOT_CONNECTIVITY_LIBRARY_ATTRIBUTES_H__
#define __IOT_CONNECTIVITY_LIBRARY_ATTRIBUTES_H__

#include "icl-value.h"
#include "icl-representation.h"

int icl_attributes_set_value(iotcon_attributes_h attributes, const char *key,
		iotcon_value_h value);

void icl_attributes_clone_foreach(char *key, iotcon_value_h src_val,
		iotcon_attributes_h dest_attributes);

iotcon_attributes_h icl_attributes_ref(iotcon_attributes_h attributes);

#endif /* __IOT_CONNECTIVITY_LIBRARY_ATTRIBUTES_H__ */
