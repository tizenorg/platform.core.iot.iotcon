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
#ifndef __IOT_CONNECTIVITY_MANAGER_DAEMON_PAYLOAD_H__
#define __IOT_CONNECTIVITY_MANAGER_DAEMON_PAYLOAD_H__

#include <glib.h>

#include <ocpayload.h>

GVariant* icd_payload_representation_empty_gvariant(void);
GVariant* icd_payload_to_gvariant(OCPayload *payload);
GVariant** icd_payload_res_to_gvariant(OCPayload *payload, OCDevAddr *dev_addr);
OCRepPayload* icd_payload_representation_from_gvariant(GVariant *var);

#endif /*__IOT_CONNECTIVITY_MANAGER_DAEMON_PAYLOAD_H__*/
