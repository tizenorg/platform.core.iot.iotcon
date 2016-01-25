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
#include "ic-common.h"
#include "ic-log.h"
#include "ic-utils.h"
#include "ic-ioty-utils.h"

OCConnectivityType ic_ioty_utils_convert_conn_type(int conn_type)
{
	OCConnectivityType oic_conn_type = CT_DEFAULT;

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		oic_conn_type = CT_ADAPTER_IP | CT_IP_USE_V4;
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		oic_conn_type = CT_ADAPTER_IP | CT_IP_USE_V6;
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

int ic_ioty_utils_parse_oic_transport(OCTransportAdapter adapter,
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


int ic_ioty_utils_convert_dev_address(const char *host_address, int conn_type, OCDevAddr *dev_addr)
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


int ic_ioty_utils_parse_oic_dev_address(OCDevAddr *dev_addr, char **host_address, int *conn_type)
{
	int connectivity_type;
	char host_addr[PATH_MAX] = {0};

	connectivity_type = ic_ioty_utils_parse_oic_transport(dev_addr->adapter,
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


int ic_ioty_utils_parse_oic_properties(int oic_properties)
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

int ic_ioty_utils_parse_oic_error(int ret)
{
	switch (ret) {
	case OC_STACK_NO_MEMORY:
		return IOTCON_ERROR_OUT_OF_MEMORY;
	case OC_STACK_NO_RESOURCE:
		return IOTCON_ERROR_NO_DATA;
	default:
		break;
	}
	return IOTCON_ERROR_IOTIVITY;
}

int ic_ioty_utils_convert_trigger(OCPresenceTrigger src, iotcon_presence_trigger_e *dest)
{
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	switch (src) {
	case OC_PRESENCE_TRIGGER_CREATE:
		*dest = IOTCON_PRESENCE_RESOURCE_CREATED;
		break;
	case OC_PRESENCE_TRIGGER_CHANGE:
		*dest = IOTCON_PRESENCE_RESOURCE_UPDATED;
		break;
	case OC_PRESENCE_TRIGGER_DELETE:
		*dest = IOTCON_PRESENCE_RESOURCE_DESTROYED;
		break;
	default:
		ERR("Invalid trigger(%d)", src);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

int ic_ioty_utils_parse_oic_result(OCStackResult result)
{
	int res;

	switch (result) {
	case OC_STACK_OK:
		res = IOTCON_RESPONSE_OK;
		break;
	case OC_STACK_RESOURCE_CREATED:
		res = IOTCON_RESPONSE_RESOURCE_CREATED;
		break;
	case OC_STACK_RESOURCE_DELETED:
		res = IOTCON_RESPONSE_RESOURCE_DELETED;
		break;
	case OC_STACK_UNAUTHORIZED_REQ:
		res = IOTCON_RESPONSE_FORBIDDEN;
		break;
	default:
		WARN("response error(%d)", result);
		res = IOTCON_RESPONSE_ERROR;
		break;
	}

	return res;
}

OCMethod ic_ioty_utils_convert_request_type(iotcon_request_type_e type)
{
	switch (type) {
	case IOTCON_REQUEST_GET:
		return OC_REST_GET;
	case IOTCON_REQUEST_PUT:
		return OC_REST_PUT;
	case IOTCON_REQUEST_POST:
		return OC_REST_POST;
	case IOTCON_REQUEST_DELETE:
		return OC_REST_DELETE;
	default:
		ERR("Invalid CRUD Type(%d)", type);
		return OC_REST_NOMETHOD;
	}
}

