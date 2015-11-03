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
#include <octypes.h>

#include "iotcon-constant.h"
#include "icd.h"
#include "icd-ioty-type.h"

OCConnectivityType icd_ioty_conn_type_to_oic_conn_type(int conn_type)
{
	OCConnectivityType oic_conn_type = CT_DEFAULT;

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		oic_conn_type = CT_IP_USE_V4;
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		oic_conn_type = CT_IP_USE_V6;
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
		oic_conn_type = CT_ADAPTER_RFCOMM_BTEDR;
		break;
	case IOTCON_CONNECTIVITY_BT_LE:
		oic_conn_type = CT_ADAPTER_GATT_BTLE;
		break;
	case IOTCON_CONNECTIVITY_ALL:
	default:
		oic_conn_type = CT_DEFAULT;
	}

	return oic_conn_type;
}


int icd_ioty_transport_flag_to_conn_type(OCTransportAdapter adapter,
		OCTransportFlags flag)
{
	int conn_type = IOTCON_CONNECTIVITY_ALL;

	/* Need to consider to allow various connectivity types */
	switch (adapter) {
	case OC_ADAPTER_IP:
		if (OC_IP_USE_V4 == flag)
			conn_type = IOTCON_CONNECTIVITY_IPV4;
		else if (OC_IP_USE_V6 == flag)
			conn_type = IOTCON_CONNECTIVITY_IPV6;
		break;
	case OC_ADAPTER_RFCOMM_BTEDR:
		conn_type = IOTCON_CONNECTIVITY_BT_EDR;
		break;
	case OC_ADAPTER_GATT_BTLE:
		conn_type = IOTCON_CONNECTIVITY_BT_LE;
		break;
	default:
		ERR("Invalid Adpater");
	}

	return conn_type;
}


int icd_ioty_conn_type_to_oic_transport_type(int conn_type, OCTransportAdapter *adapter,
		OCTransportFlags *flag)
{
	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		*adapter = OC_ADAPTER_IP;
		*flag = OC_IP_USE_V4;
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		*adapter = OC_ADAPTER_IP;
		*flag = OC_IP_USE_V6;
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
		*adapter = OC_ADAPTER_RFCOMM_BTEDR;
		*flag = OC_DEFAULT_FLAGS;
		break;
	case IOTCON_CONNECTIVITY_BT_LE:
		*adapter = OC_ADAPTER_GATT_BTLE;
		*flag = OC_DEFAULT_FLAGS;
		break;
	case IOTCON_CONNECTIVITY_ALL:
	default:
		*adapter = OC_DEFAULT_ADAPTER;
		*flag = OC_DEFAULT_FLAGS;
	}

	return IOTCON_ERROR_NONE;
}

