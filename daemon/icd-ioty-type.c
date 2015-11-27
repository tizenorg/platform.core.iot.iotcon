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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <octypes.h>

#include "iotcon-types.h"
#include "ic-utils.h"
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
		if (OC_IP_USE_V4 & flag)
			conn_type = IOTCON_CONNECTIVITY_IPV4;
		else if (OC_IP_USE_V6 & flag)
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


static void _icd_ioty_conn_type_to_oic_transport_type(int conn_type,
		OCTransportAdapter *adapter, OCTransportFlags *flag)
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
}


int icd_ioty_get_dev_addr(const char *host_address, int conn_type, OCDevAddr *dev_addr)
{
	char host[PATH_MAX] = {0};
	char *dev_host, *ptr = NULL;

	snprintf(host, sizeof(host), "%s", host_address);

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		dev_host = strtok_r(host, ":", &ptr);
		snprintf(dev_addr->addr, sizeof(dev_addr->addr), "%s", dev_host);
		dev_addr->port = atoi(strtok_r(NULL, ":", &ptr));
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		dev_host = strtok_r(host, "]", &ptr);
		snprintf(dev_addr->addr, sizeof(dev_addr->addr), "%s", dev_host);
		dev_addr->port = atoi(strtok_r(NULL, "]", &ptr));
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
		snprintf(dev_addr->addr, sizeof(dev_addr->addr), "%s", host);
		break;
	case IOTCON_CONNECTIVITY_BT_LE:
	default:
		ERR("Invalid Connectivity Type(%d)", conn_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	_icd_ioty_conn_type_to_oic_transport_type(conn_type, &(dev_addr->adapter),
			&(dev_addr->flags));

	return IOTCON_ERROR_NONE;
}


int icd_ioty_get_host_address(OCDevAddr *dev_addr, char **host_address, int *conn_type)
{
	int connectivity_type;
	char host_addr[PATH_MAX] = {0};

	connectivity_type = icd_ioty_transport_flag_to_conn_type(dev_addr->adapter,
			dev_addr->flags);

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV6:
		snprintf(host_addr, sizeof(host_addr), "[%s]:%d", dev_addr->addr,
				dev_addr->port);
		break;
	case IOTCON_CONNECTIVITY_IPV4:
		snprintf(host_addr, sizeof(host_addr), "%s:%d", dev_addr->addr,
				dev_addr->port);
		break;
	case IOTCON_CONNECTIVITY_BT_EDR:
		snprintf(host_addr, sizeof(host_addr), "%s", dev_addr->addr);
		break;
	case IOTCON_CONNECTIVITY_BT_LE:
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*host_address = strdup(host_addr);

	*conn_type = connectivity_type;

	return IOTCON_ERROR_NONE;
}


int icd_ioty_oic_properties_to_properties(int oic_properties)
{
	int prop = IOTCON_RESOURCE_NO_PROPERTY;

	if (OC_DISCOVERABLE & oic_properties)
		prop |= IOTCON_RESOURCE_DISCOVERABLE;

	if (OC_OBSERVABLE & oic_properties)
		prop |= IOTCON_RESOURCE_OBSERVABLE;

	if (OC_ACTIVE & oic_properties)
		prop |= IOTCON_RESOURCE_ACTIVE;

	if (OC_SLOW & oic_properties)
		prop |= IOTCON_RESOURCE_SLOW;

	if (OC_SECURE & oic_properties)
		prop |= IOTCON_RESOURCE_SECURE;

	if (OC_EXPLICIT_DISCOVERABLE & oic_properties)
		prop |= IOTCON_RESOURCE_EXPLICIT_DISCOVERABLE;

	return prop;
}
