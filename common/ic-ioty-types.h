/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __IOT_CONNECTIVITY_COMMON_IOTIVITY_TYPES_H__
#define __IOT_CONNECTIVITY_COMMON_IOTIVITY_TYPES_H__

#include <octypes.h>

/* oic -> iotcon */
iotcon_connectivity_type_e ic_ioty_parse_oic_transport(OCTransportAdapter adapter,
		OCTransportFlags flag);
iotcon_request_type_e ic_ioty_parse_oic_method(OCMethod method);
iotcon_response_result_e ic_ioty_parse_oic_response_result(OCStackResult result);
uint8_t ic_ioty_parse_oic_policies(uint8_t policies);
iotcon_error_e ic_ioty_parse_oic_error(OCStackResult ret);
iotcon_presence_trigger_e ic_ioty_parse_oic_trigger(OCPresenceTrigger src);
iotcon_observe_type_e ic_ioty_parse_oic_action(OCObserveAction oic_action);
int ic_ioty_parse_oic_dev_address(OCDevAddr *dev_addr, char **host_address,
		int *conn_type);

/* iotcon -> oic */
OCConnectivityType ic_ioty_convert_connectivity_type(
		iotcon_connectivity_type_e conn_type);
OCMethod ic_ioty_convert_request_type(iotcon_request_type_e req_type);
OCEntityHandlerResult ic_ioty_convert_response_result(iotcon_response_result_e result);
OCQualityOfService ic_ioty_convert_qos(iotcon_qos_e qos);
int ic_ioty_convert_connectivity(const char *host_address, int conn_type,
		OCDevAddr *dev_addr);
uint8_t ic_ioty_convert_policies(uint8_t policies);

#endif /*__IOT_CONNECTIVITY_COMMON_IOTIVITY_TYPES_H__*/
