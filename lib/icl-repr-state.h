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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_STATE_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_STATE_H__

#include "iotcon-struct.h"
#include "iotcon-constant.h"
#include "icl-repr-value.h"

void icl_state_inc_ref_count(iotcon_state_h val);
bool icl_state_dec_ref_count(iotcon_state_h val);

int icl_state_del_value(iotcon_state_h state, const char *key,
		iotcon_types_e value_type);

int icl_state_set_value(iotcon_state_h state, const char *key, iotcon_value_h value);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_STATE_H__ */
