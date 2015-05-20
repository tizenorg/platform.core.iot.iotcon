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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_IOTIVITY_REPRESENTATION_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_IOTIVITY_REPRESENTATION_H__

#include <OCRepresentation.h>
#include "iotcon-struct.h"

void ic_ioty_repr_found_device_cb(const OC::OCRepresentation& ocRep);
iotcon_repr_h ic_ioty_repr_generate_repr(const OC::OCRepresentation& ocRep);
OC::OCRepresentation ic_ioty_repr_parse(iotcon_repr_h repr);

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_IOTIVITY_REPRESENTATION_H__ */

