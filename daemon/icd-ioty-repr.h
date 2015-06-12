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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_REPRESENTATION_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_REPRESENTATION_H__

#include <OCRepresentation.h>
#include <glib.h>
#include "iotcon-struct.h"

int icd_ioty_repr_parse_json(const char *repr_json, OC::OCRepresentation &ocRep);
int icd_ioty_repr_generate_gvariant_builder(const OC::OCRepresentation &ocRep,
		GVariantBuilder **builder);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_IOTIVITY_REPRESENTATION_H__ */

