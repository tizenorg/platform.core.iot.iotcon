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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_PAYLOAD_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_PAYLOAD_H__

#include <glib.h>

void icl_state_from_gvariant(iotcon_state_h state, GVariantIter *iter);
GVariant* icl_representation_to_gvariant(iotcon_representation_h repr);
iotcon_representation_h icl_representation_from_gvariant(GVariant *var);

#endif /*__IOT_CONNECTIVITY_MANAGER_LIBRARY_PAYLOAD_H__*/
