/*
 * Copyright (c) 2015 - 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <limits.h>

#include <octypes.h>

#include "iotcon-types.h"
#include "iotcon-options.h"
#include "ic-common.h"
#include "ic-log.h"
#include "ic-utils.h"
#include "ic-ioty-types.h"

OCConnectivityType ic_ioty_convert_connectivity_type(
		iotcon_connectivity_type_e conn_type)
{
	int connectivity_type = conn_type;

	switch (connectivity_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		return CT_ADAPTER_IP | CT_IP_USE_V4;
	case IOTCON_CONNECTIVITY_IPV6:
		return CT_ADAPTER_IP | CT_IP_USE_V6;
	case IOTCON_CONNECTIVITY_ALL:
		return CT_DEFAULT;
	default:
		ERR("Invalid conn_type(%d)", conn_type);
	}
	return CT_DEFAULT;
}

iotcon_connectivity_type_e ic_ioty_parse_oic_transport(
		OCTransportAdapter adapter, OCTransportFlags flag)
{
	iotcon_connectivity_type_e type = IOTCON_CONNECTIVITY_ALL;

	/* Need to consider to allow various connectivity types */
	switch (adapter) {
	case OC_ADAPTER_IP:
		if (OC_IP_USE_V4 & flag)
			type = IOTCON_CONNECTIVITY_IPV4;
		else if (OC_IP_USE_V6 & flag)
			type = IOTCON_CONNECTIVITY_IPV6;
		else
			ERR("Invalid Flag(%d)", flag);
		break;
	default:
		ERR("Invalid Adpater(%d)", adapter);
	}
	return type;
}


static void _ic_ioty_conn_type_to_oic_transport_type(int conn_type,
		OCTransportAdapter *adapter, OCTransportFlags *flag)
{
	RET_IF(NULL == adapter);
	RET_IF(NULL == flag);

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		*adapter = OC_ADAPTER_IP;
		*flag = OC_IP_USE_V4;
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		*adapter = OC_ADAPTER_IP;
		*flag = OC_IP_USE_V6;
		break;
	case IOTCON_CONNECTIVITY_ALL:
	default:
		*adapter = OC_DEFAULT_ADAPTER;
		*flag = OC_DEFAULT_FLAGS;
	}
}


int ic_ioty_convert_connectivity(const char *host_address, int conn_type,
		OCDevAddr *dev_addr)
{
	char host[PATH_MAX] = {0};
	char *dev_host, *ptr = NULL;

	RETV_IF(NULL == dev_addr, IOTCON_ERROR_INVALID_PARAMETER);

	snprintf(host, sizeof(host), "%s", host_address);

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		dev_host = strtok_r(host, ":", &ptr);
		snprintf(dev_addr->addr, sizeof(dev_addr->addr), "%s", dev_host);
		dev_addr->port = atoi(strtok_r(NULL, ":", &ptr));
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		dev_host = strtok_r(host, "]", &ptr);
		snprintf(dev_addr->addr, sizeof(dev_addr->addr), "%s", dev_host + 1);
		dev_addr->port = atoi(strtok_r(NULL, ":", &ptr));
		break;
	default:
		ERR("Invalid Connectivity Type(%d)", conn_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	_ic_ioty_conn_type_to_oic_transport_type(conn_type, &(dev_addr->adapter),
			&(dev_addr->flags));

	return IOTCON_ERROR_NONE;
}


int ic_ioty_parse_oic_dev_address(OCDevAddr *dev_addr, char **host_address,
		int *conn_type)
{
	int connectivity_type;
	char host_addr[PATH_MAX] = {0};

	RETV_IF(NULL == host_address, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == conn_type, IOTCON_ERROR_INVALID_PARAMETER);

	connectivity_type = ic_ioty_parse_oic_transport(dev_addr->adapter,
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
	default:
		ERR("Invalid Connectivity Type(%d)", connectivity_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*host_address = strdup(host_addr);

	*conn_type = connectivity_type;

	return IOTCON_ERROR_NONE;
}


int ic_ioty_parse_oic_properties(int oic_properties)
{
	int policies = IOTCON_RESOURCE_NO_PROPERTY;

	if (OC_DISCOVERABLE & oic_properties)
		policies |= IOTCON_RESOURCE_DISCOVERABLE;

	if (OC_OBSERVABLE & oic_properties)
		policies |= IOTCON_RESOURCE_OBSERVABLE;

	if (OC_ACTIVE & oic_properties)
		policies |= IOTCON_RESOURCE_ACTIVE;

	if (OC_SLOW & oic_properties)
		policies |= IOTCON_RESOURCE_SLOW;

	if (OC_SECURE & oic_properties)
		policies |= IOTCON_RESOURCE_SECURE;

	if (OC_EXPLICIT_DISCOVERABLE & oic_properties)
		policies |= IOTCON_RESOURCE_EXPLICIT_DISCOVERABLE;

	return policies;
}

iotcon_error_e ic_ioty_parse_oic_error(OCStackResult ret)
{
	switch (ret) {
	case OC_STACK_NO_MEMORY:
		return IOTCON_ERROR_OUT_OF_MEMORY;
	case OC_STACK_NO_RESOURCE:
		return IOTCON_ERROR_NO_DATA;
	default:
		WARN("Unknown result(%d)", ret);
	}
	return IOTCON_ERROR_IOTIVITY;
}

iotcon_presence_trigger_e ic_ioty_parse_oic_trigger(OCPresenceTrigger src)
{
	switch (src) {
	case OC_PRESENCE_TRIGGER_CREATE:
		return IOTCON_PRESENCE_RESOURCE_CREATED;
	case OC_PRESENCE_TRIGGER_CHANGE:
		return IOTCON_PRESENCE_RESOURCE_UPDATED;
	case OC_PRESENCE_TRIGGER_DELETE:
		return IOTCON_PRESENCE_RESOURCE_DESTROYED;
	default:
		ERR("Unknown trigger(%d)", src);
	}
	return IOTCON_PRESENCE_RESOURCE_CREATED;
}

OCEntityHandlerResult ic_ioty_convert_response_result(
		iotcon_response_result_e result)
{
	switch (result) {
	case IOTCON_RESPONSE_OK:
		return OC_EH_OK;
	case IOTCON_RESPONSE_RESOURCE_CREATED:
		return OC_EH_RESOURCE_CREATED;
	case IOTCON_RESPONSE_RESOURCE_DELETED:
		return OC_EH_RESOURCE_DELETED;
	case IOTCON_RESPONSE_FORBIDDEN:
		return OC_EH_FORBIDDEN;
	case IOTCON_RESPONSE_ERROR:
		return OC_EH_ERROR;
	default:
		WARN("Unknown result(%d)", result);
	}
	return OC_EH_ERROR;
}

iotcon_response_result_e  ic_ioty_parse_oic_response_result(OCStackResult result)
{
	switch (result) {
	case OC_STACK_OK:
		return IOTCON_RESPONSE_OK;
	case OC_STACK_RESOURCE_CREATED:
		return IOTCON_RESPONSE_RESOURCE_CREATED;
	case OC_STACK_RESOURCE_DELETED:
		return IOTCON_RESPONSE_RESOURCE_DELETED;
	case OC_STACK_UNAUTHORIZED_REQ:
		return IOTCON_RESPONSE_FORBIDDEN;
	case OC_STACK_ERROR:
		return IOTCON_RESPONSE_ERROR;
	default:
		WARN("Unknown result(%d)", result);
	}
	return IOTCON_RESPONSE_ERROR;
}

OCMethod ic_ioty_convert_request_type(iotcon_request_type_e type)
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
		ERR("Unknown Type(%d)", type);
	}
	return OC_REST_NOMETHOD;
}

iotcon_request_type_e ic_ioty_parse_oic_method(OCMethod method)
{
	switch (method) {
	case OC_REST_GET:
		return IOTCON_REQUEST_GET;
	case OC_REST_PUT:
		return IOTCON_REQUEST_PUT;
	case OC_REST_POST:
		return IOTCON_REQUEST_POST;
	case OC_REST_DELETE:
		return IOTCON_REQUEST_DELETE;
	default:
		ERR("Unknown method(%d)", method);
		return IOTCON_REQUEST_UNKNOWN;
	}
}

iotcon_observe_type_e ic_ioty_parse_oic_action(OCObserveAction oic_action)
{
	switch (oic_action) {
	case OC_OBSERVE_REGISTER:
		return IOTCON_OBSERVE_REGISTER;
	case OC_OBSERVE_DEREGISTER:
		return IOTCON_OBSERVE_DEREGISTER;
	case OC_OBSERVE_NO_OPTION:
	default:
		ERR("Unknown action (%d)", oic_action);
	}
	return IOTCON_OBSERVE_NO_TYPE;
}

OCQualityOfService ic_ioty_convert_qos(iotcon_qos_e qos)
{
	switch (qos) {
	case IOTCON_QOS_HIGH:
		return OC_HIGH_QOS;
	case IOTCON_QOS_LOW:
		return OC_LOW_QOS;
	default:
		ERR("Unknown qos(%d)", qos);
	}
	return OC_NA_QOS;
}

int ic_ioty_convert_policies(int policies)
{
	int prop = OC_RES_PROP_NONE;

	if (IOTCON_RESOURCE_DISCOVERABLE & policies)
		prop |= OC_DISCOVERABLE;

	if (IOTCON_RESOURCE_OBSERVABLE & policies)
		prop |= OC_OBSERVABLE;

	if (IOTCON_RESOURCE_ACTIVE & policies)
		prop |= OC_ACTIVE;

	if (IOTCON_RESOURCE_SLOW & policies)
		prop |= OC_SLOW;

	if (IOTCON_RESOURCE_SECURE & policies)
		prop |= OC_SECURE;

	if (IOTCON_RESOURCE_EXPLICIT_DISCOVERABLE & policies)
		prop |= OC_EXPLICIT_DISCOVERABLE;

	return prop;
}

