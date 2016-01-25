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
#ifndef __IOT_CONNECTIVITY_MANAGER_COMMON_IOTIVITY_UTILS_H__
#define __IOT_CONNECTIVITY_MANAGER_COMMON_IOTIVITY_UTILS_H__

#include <octypes.h>

/* parse : oic type -> ioty type */
/* convert : ioty type -> oic type */

OCConnectivityType ic_ioty_utils_convert_conn_type(int conn_type);
int ic_ioty_utils_parse_oic_transport(OCTransportAdapter adapter, OCTransportFlags flag);
int ic_ioty_utils_convert_dev_address(const char *host_address, int conn_type, OCDevAddr *dev_addr);
int ic_ioty_utils_parse_oic_dev_address(OCDevAddr *dev_addr, char **host_address, int *conn_type);
int ic_ioty_utils_parse_oic_properties(int oic_properties);
int ic_ioty_utils_parse_oic_error(int ret);
int ic_ioty_utils_convert_trigger(OCPresenceTrigger src, iotcon_presence_trigger_e *dest);
int ic_ioty_utils_parse_oic_result(OCStackResult result);
OCMethod ic_ioty_utils_convert_request_type(iotcon_request_type_e req_type);

#endif /*__IOT_CONNECTIVITY_MANAGER_COMMON_IOTIVITY_UTILS_H__*/
