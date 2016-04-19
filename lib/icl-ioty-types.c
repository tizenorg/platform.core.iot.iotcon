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

#include <stdlib.h>
#include <stdint.h>
#include <octypes.h>
#include <ocstack.h>
#include <ocpayload.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "ic-ioty-types.h"
#include "icl.h"
#include "icl-device.h"
#include "icl-remote-resource.h"
#include "icl-list.h"
#include "icl-value.h"
#include "icl-presence.h"
#include "icl-representation.h"
#include "icl-resource-types.h"
#include "icl-resource-interfaces.h"
#include "icl-ioty.h"
#include "icl-types.h"
#include "icl-ioty-types.h"

struct icl_state_list_s {
	OCRepPayloadPropType type;
	size_t dimensions[MAX_REP_ARRAY_DEPTH];
	GList *list;
};

static int _icl_ioty_parse_oic_rep_payload_value(OCRepPayloadValue *val,
		iotcon_state_h *state);
static int _icl_ioty_fill_oic_rep_payload_value(OCRepPayload *payload,
		iotcon_state_h state);

static void _icl_ioty_free_resource_list(iotcon_remote_resource_h *resource_list,
		int resource_count)
{
	int i;

	RET_IF(NULL == resource_list);

	for (i = 0; i < resource_count; i++)
		iotcon_remote_resource_destroy(resource_list[i]);

	free(resource_list);
}

int icl_ioty_parse_oic_discovery_payload(OCDevAddr *dev_addr,
		OCDiscoveryPayload *payload,
		iotcon_remote_resource_h **resource_list,
		int *resource_count)
{
	int i, ret, res_count;
	iotcon_remote_resource_h *res_list;
	struct OCResourcePayload *res_payload;

	RETV_IF(NULL == payload, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dev_addr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == resource_count, IOTCON_ERROR_INVALID_PARAMETER);

	res_payload = payload->resources;
	res_count = OCDiscoveryPayloadGetResourceCount(payload);
	if (res_count <= 0) {
		ERR("Invalid count(%d)", res_count);
		return IOTCON_ERROR_IOTIVITY;
	}

	res_list = calloc(res_count, sizeof(iotcon_remote_resource_h));
	if (NULL == res_list) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	for (i = 0; res_payload; i++, res_payload = res_payload->next) {
		int port, properties, conn_type;
		iotcon_resource_interfaces_h ifaces;
		iotcon_resource_types_h types;
		char host_addr[PATH_MAX] = {0};
		OCStringLL *node;

		/* uri path */
		if (NULL == res_payload->uri) {
			ERR("res_payload uri is NULL");
			_icl_ioty_free_resource_list(res_list, res_count);
			return IOTCON_ERROR_IOTIVITY;
		}

		/* Resource Types */
		node = res_payload->types;
		if (NULL == node) {
			ERR("res_payload types is NULL");
			_icl_ioty_free_resource_list(res_list, res_count);
			return IOTCON_ERROR_IOTIVITY;
		}

		ret = iotcon_resource_types_create(&types);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_resource_types_create() Fail(%d)", ret);
			_icl_ioty_free_resource_list(res_list, res_count);
			return ret;
		}

		while (node) {
			ret = iotcon_resource_types_add(types, node->value);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_resource_types_add() Fail(%d)", ret);
				iotcon_resource_types_destroy(types);
				_icl_ioty_free_resource_list(res_list, res_count);
				return ret;
			}
			node = node->next;
		}

		/* Resource Interfaces */
		iotcon_resource_interfaces_create(&ifaces);
		node = res_payload->interfaces;
		if (NULL == node) {
			ERR("res_payload interfaces is NULL");
			iotcon_resource_types_destroy(types);
			continue;
		}
		for (; node; node = node->next)
			iotcon_resource_interfaces_add(ifaces, node->value);

		/* Resource Properties */
		properties = ic_ioty_parse_oic_properties(res_payload->bitmap);
		if (res_payload->secure)
			properties |= IOTCON_RESOURCE_SECURE;

		/* port */
		port = (res_payload->port) ? res_payload->port : dev_addr->port;

		/* connectivity type */
		conn_type = ic_ioty_parse_oic_transport(dev_addr->adapter, dev_addr->flags);

		/* host address */
		switch (conn_type) {
		case IOTCON_CONNECTIVITY_IPV6:
			snprintf(host_addr, sizeof(host_addr), "[%s]:%d", dev_addr->addr, port);
			break;
		case IOTCON_CONNECTIVITY_IPV4:
			snprintf(host_addr, sizeof(host_addr), "%s:%d", dev_addr->addr, port);
			break;
		default:
			snprintf(host_addr, sizeof(host_addr), "%s", dev_addr->addr);
		}

		ret = iotcon_remote_resource_create(host_addr, conn_type, res_payload->uri,
				properties, types, ifaces, &(res_list[i]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_remote_resource_create() Fail(%d)", ret);
			iotcon_resource_interfaces_destroy(ifaces);
			iotcon_resource_types_destroy(types);
			_icl_ioty_free_resource_list(res_list, res_count);
			return ret;
		}

		res_list[i]->device_id = strdup(payload->sid);
		if (NULL == res_list[i]->device_id) {
			ERR("strdup(device_id) Fail(%d)", errno);
			_icl_ioty_free_resource_list(res_list, res_count);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
		iotcon_resource_interfaces_destroy(ifaces);
		iotcon_resource_types_destroy(types);
	}

	*resource_list = res_list;
	*resource_count = res_count;
	return IOTCON_ERROR_NONE;

}

int icl_ioty_parse_oic_device_payload(OCDevicePayload *payload,
		iotcon_device_info_h *device_info)
{
	struct icl_device_info *info = NULL;

	RETV_IF(NULL == device_info, IOTCON_ERROR_INVALID_PARAMETER);

	info = calloc(1, sizeof(struct icl_device_info));
	if (NULL == info) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	info->device_name = ic_utils_strdup(payload->deviceName);
	info->spec_ver = ic_utils_strdup(payload->specVersion);
	info->data_model_ver = ic_utils_strdup(payload->dataModelVersion);
	info->device_id = ic_utils_strdup(payload->sid);

	*device_info = info;
	return IOTCON_ERROR_NONE;
}

int icl_ioty_parse_oic_platform_payload(OCPlatformPayload *payload,
		iotcon_platform_info_h *platform_info)
{
	struct icl_platform_info *info = NULL;

	RETV_IF(NULL == payload, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == platform_info, IOTCON_ERROR_INVALID_PARAMETER);

	info = calloc(1, sizeof(struct icl_platform_info));
	if (NULL == info) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	info->platform_id  = ic_utils_strdup(payload->info.platformID);
	info->manuf_name = ic_utils_strdup(payload->info.manufacturerName);
	if (payload->info.manufacturerUrl)
		info->manuf_url = ic_utils_strdup(payload->info.manufacturerUrl);
	if (payload->info.modelNumber)
		info->model_number = ic_utils_strdup(payload->info.modelNumber);
	if (payload->info.dateOfManufacture)
		info->date_of_manuf = ic_utils_strdup(payload->info.dateOfManufacture);
	if (payload->info.platformVersion)
		info->platform_ver = ic_utils_strdup(payload->info.platformVersion);
	if (payload->info.operatingSystemVersion)
		info->os_ver = ic_utils_strdup(payload->info.operatingSystemVersion);
	if (payload->info.hardwareVersion)
		info->hardware_ver = ic_utils_strdup(payload->info.hardwareVersion);
	if (payload->info.firmwareVersion)
		info->firmware_ver = ic_utils_strdup(payload->info.firmwareVersion);
	if (payload->info.supportUrl)
		info->support_url = ic_utils_strdup(payload->info.supportUrl);
	if (payload->info.systemTime)
		info->system_time = ic_utils_strdup(payload->info.systemTime);

	*platform_info = info;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_parse_oic_presence_payload(OCDevAddr *dev_addr,
		OCPresencePayload *payload,
		OCStackResult result,
		iotcon_presence_response_h *presence_response)
{
	FN_CALL;
	int ret;
	struct icl_presence_response *resp = NULL;

	RETV_IF(NULL == dev_addr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == payload, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == presence_response, IOTCON_ERROR_INVALID_PARAMETER);

	resp = calloc(1, sizeof(struct icl_presence_response));
	if (NULL == resp) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	ret = ic_ioty_parse_oic_dev_address(dev_addr, &(resp->host_address),
			(int*)&(resp->connectivity_type));
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_parse_oic_dev_address() Fail(%d)", ret);
		icl_destroy_presence_response(resp);
		return ret;
	}

	switch (result) {
	case OC_STACK_OK:
		resp->result = IOTCON_PRESENCE_OK;
		resp->trigger = ic_ioty_parse_oic_trigger(payload->trigger);
		break;
	case OC_STACK_PRESENCE_STOPPED:
		resp->result = IOTCON_PRESENCE_STOPPED;
		break;
	case OC_STACK_PRESENCE_TIMEOUT:
		resp->result = IOTCON_PRESENCE_TIMEOUT;
		break;
	case OC_STACK_ERROR:
	default:
		DBG("Presence error(%d)", result);
		resp->result = IOTCON_ERROR_IOTIVITY;
	}
	if (payload->resourceType)
		resp->resource_type = strdup(payload->resourceType);

	*presence_response = resp;

	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_parse_oic_rep_payload_value_array_attr(
		OCRepPayloadValueArray *arr, int len, int index, iotcon_list_h *list)
{
	int i, ret;
	iotcon_list_h l = NULL;

	RETV_IF(NULL == arr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	switch (arr->type) {
	case OCREP_PROP_INT:
		ret = iotcon_list_create(IOTCON_TYPE_INT, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create(IOTCON_TYPE_INT) Fail(%d)", ret);
			return ret;
		}
		for (i = 0; i < len; i++) {
			ret = iotcon_list_add_int(l, arr->iArray[index + i], -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_list_add_int() Fail(%d)", ret);
				iotcon_list_destroy(l);
				return ret;
			}
		}
		break;
	case OCREP_PROP_BOOL:
		ret = iotcon_list_create(IOTCON_TYPE_BOOL, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create(IOTCON_TYPE_BOOL) Fail(%d)", ret);
			return ret;
		}
		for (i = 0; i < len; i++) {
			ret = iotcon_list_add_bool(l, arr->bArray[index + i], -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_list_add_bool() Fail(%d)", ret);
				iotcon_list_destroy(l);
				return ret;
			}
		}
		break;
	case OCREP_PROP_DOUBLE:
		ret = iotcon_list_create(IOTCON_TYPE_DOUBLE, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create(IOTCON_TYPE_DOUBLE) Fail(%d)", ret);
			return ret;
		}
		for (i = 0; i < len; i++) {
			ret = iotcon_list_add_double(l, arr->dArray[index + i], -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_list_add_double() Fail(%d)", ret);
				iotcon_list_destroy(l);
				return ret;
			}
		}
		break;
	case OCREP_PROP_STRING:
		ret = iotcon_list_create(IOTCON_TYPE_STR, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create(IOTCON_TYPE_STR) Fail(%d)", ret);
			return ret;
		}
		for (i = 0; i < len; i++) {
			ret = iotcon_list_add_str(l, arr->strArray[index + i], -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_list_add_str() Fail(%d)", ret);
				iotcon_list_destroy(l);
				return ret;
			}
		}
		break;
	case OCREP_PROP_BYTE_STRING:
		ret = iotcon_list_create(IOTCON_TYPE_BYTE_STR, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create(IOTCON_TYPE_BYTE_STR) Fail(%d)", ret);
			return ret;
		}
		for (i = 0; i < len; i++) {
			ret = iotcon_list_add_byte_str(l, arr->ocByteStrArray[index + i].bytes,
					arr->ocByteStrArray[index + i].len, -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_list_add_byte_str() Fail(%d)", ret);
				iotcon_list_destroy(l);
				return ret;
			}
		}
		break;
	case OCREP_PROP_OBJECT:
		ret = iotcon_list_create(IOTCON_TYPE_STATE, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_create(IOTCON_TYPE_STATE) Fail(%d)", ret);
			return ret;
		}
		for (i = 0; i < len; i++) {
			iotcon_state_h state;
			ret = _icl_ioty_parse_oic_rep_payload_value(arr->objArray[index + i]->values,
					&state);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_parse_oic_rep_payload_value(%d)", ret);
				iotcon_list_destroy(l);
				return ret;
			}

			ret = iotcon_list_add_state(l, state, -1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_list_add_state() Fail(%d)", ret);
				iotcon_state_destroy(state);
				iotcon_list_destroy(l);
				return ret;
			}
			iotcon_state_destroy(state);
		}
		break;
	case OCREP_PROP_ARRAY:
	case OCREP_PROP_NULL:
	default:
		break;
	}

	*list = l;
	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_parse_oic_rep_payload_value_array(
		OCRepPayloadValueArray *arr, int depth, int len, int index, iotcon_list_h *list)
{
	int i, ret, next_len;
	iotcon_list_h l;

	RETV_IF(NULL == arr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	if ((MAX_REP_ARRAY_DEPTH - 1) == depth || 0 == arr->dimensions[depth + 1]) {
		ret = _icl_ioty_parse_oic_rep_payload_value_array_attr(arr, len, index, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_ioty_parse_oic_rep_payload_value_array_attr() Fail(%d)", ret);
			return ret;
		}
		*list = l;
		return IOTCON_ERROR_NONE;
	}

	i = len + index;

	next_len = len / arr->dimensions[depth];
	index -= next_len;

	ret = iotcon_list_create(IOTCON_TYPE_LIST, &l);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_list_create() Fail(%d)", ret);
		return ret;
	}

	while ((index += next_len) < i) {
		iotcon_list_h l_child = NULL;
		ret = _icl_ioty_parse_oic_rep_payload_value_array(arr, depth + 1, next_len, index,
				&l_child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_ioty_parse_oic_rep_payload_value_array() Fail(%d)", ret);
			iotcon_list_destroy(l);
			return ret;
		}

		ret = iotcon_list_add_list(l, l_child, -1);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_list_add_list() Fail(%d)", ret);
			iotcon_list_destroy(l_child);
			iotcon_list_destroy(l);
			return ret;
		}
		iotcon_list_destroy(l_child);
	}
	*list = l;
	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_parse_oic_rep_payload_value(OCRepPayloadValue *val,
		iotcon_state_h *state)
{
	int ret, total_len;
	iotcon_state_h s;
	iotcon_list_h list;
	iotcon_state_h s_obj;

	RETV_IF(NULL == val, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_state_create(&s);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_state_create() Fail(%d)", ret);
		return ret;
	}

	while (val) {
		switch (val->type) {
		case OCREP_PROP_INT:
			ret = iotcon_state_add_int(s, val->name, val->i);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_int() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			break;
		case OCREP_PROP_BOOL:
			ret = iotcon_state_add_bool(s, val->name, val->b);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_bool() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			break;
		case OCREP_PROP_DOUBLE:
			ret = iotcon_state_add_double(s, val->name, val->d);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_double() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			break;
		case OCREP_PROP_STRING:
			ret = iotcon_state_add_str(s, val->name, val->str);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_str() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			break;
		case OCREP_PROP_BYTE_STRING:
			ret = iotcon_state_add_byte_str(s, val->name, val->ocByteStr.bytes,
					val->ocByteStr.len);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_byte_str() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			break;
		case OCREP_PROP_NULL:
			ret = iotcon_state_add_null(s, val->name);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_null() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			break;
		case OCREP_PROP_ARRAY:
			total_len = calcDimTotal(val->arr.dimensions);
			ret = _icl_ioty_parse_oic_rep_payload_value_array(&val->arr, 0, total_len, 0,
					&list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_parse_oic_rep_payload_value_array() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			ret = iotcon_state_add_list(s, val->name, list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_list() Fail(%d)", ret);
				iotcon_list_destroy(list);
				iotcon_state_destroy(s);
				return ret;
			}
			iotcon_list_destroy(list);
			break;
		case OCREP_PROP_OBJECT:
			ret = _icl_ioty_parse_oic_rep_payload_value(val->obj->values, &s_obj);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_parse_oic_rep_payload_value() Fail(%d)", ret);
				iotcon_state_destroy(s);
				return ret;
			}
			ret = iotcon_state_add_state(s, val->name, s_obj);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_state_add_state() Fail(%d)", ret);
				iotcon_state_destroy(s_obj);
				iotcon_state_destroy(s);
				return ret;
			}
			iotcon_state_destroy(s_obj);
			break;
		default:
			ERR("Invalid Type(%d)", val->type);
		}
		val = val->next;
	}

	*state = s;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_parse_oic_rep_payload(OCRepPayload *payload, bool is_parent,
		iotcon_representation_h *representation)
{
	int ret = 0;
	OCStringLL *node;
	OCRepPayload *child_node;
	iotcon_resource_interfaces_h ifaces;
	iotcon_representation_h repr;
	iotcon_state_h state = NULL;
	iotcon_resource_types_h types = NULL;

	RETV_IF(NULL == payload, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == representation, IOTCON_ERROR_INVALID_PARAMETER);

	ret = iotcon_representation_create(&repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_create() Fail(%d)", ret);
		return ret;
	}

	/* uri path */
	if (payload->uri)
		repr->uri_path = strdup(payload->uri);

	/* resource types */
	if (payload->types) {
		node = payload->types;
		while (node) {
			if (NULL == types) {
				ret = iotcon_resource_types_create(&types);
				if (IOTCON_ERROR_NONE != ret) {
					ERR("iotcon_resource_types_add() Fail(%d)", ret);
					iotcon_representation_destroy(repr);
					return ret;
				}
			}
			ret = iotcon_resource_types_add(types, node->value);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("iotcon_resource_types_add() Fail(%d)", ret);
				iotcon_resource_types_destroy(types);
				iotcon_representation_destroy(repr);
				return ret;
			}
			node = node->next;
		}
		ret = iotcon_representation_set_resource_types(repr, types);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_set_resource_types() Fail(%d)", ret);
			iotcon_resource_types_destroy(types);
			iotcon_representation_destroy(repr);
			return ret;
		}
		iotcon_resource_types_destroy(types);
	}

	iotcon_resource_interfaces_create(&ifaces);

	/* resource interfaces */
	node = payload->interfaces;
	if (node) {
		while (node) {
			iotcon_resource_interfaces_add(ifaces, node->value);
			node = node->next;
		}
	} else {
		// TODO: verify spec
		iotcon_resource_interfaces_add(ifaces, IOTCON_INTERFACE_DEFAULT);
	}

	ret = iotcon_representation_set_resource_interfaces(repr, ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_resource_interfaces() Fail(%d)", ret);
		iotcon_resource_interfaces_destroy(ifaces);
		iotcon_representation_destroy(repr);
		return ret;
	}
	iotcon_resource_interfaces_destroy(ifaces);

	/* state */
	if (payload->values) {
		ret = _icl_ioty_parse_oic_rep_payload_value(payload->values, &state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_set_resource_types() Fail(%d)", ret);
			iotcon_representation_destroy(repr);
			return ret;
		}
		ret = iotcon_representation_set_state(repr, state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_set_state() Fail(%d)", ret);
			iotcon_state_destroy(state);
			iotcon_representation_destroy(repr);
			return ret;
		}
		iotcon_state_destroy(state);
	}

	/* children */
	child_node = payload->next;
	while (is_parent && child_node) {
		/* generate recursively */
		iotcon_representation_h repr_child;

		ret = icl_ioty_parse_oic_rep_payload(child_node, false, &repr_child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_parse_oic_rep_payload() Fail(%d)", ret);
			iotcon_representation_destroy(repr);
			return ret;
		}

		ret = iotcon_representation_add_child(repr, repr_child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_representation_add_child() Fail(%d)", ret);
			iotcon_representation_destroy(repr_child);
			iotcon_representation_destroy(repr);
			return ret;
		}
		iotcon_representation_destroy(repr_child);

		child_node = child_node->next;
	}

	*representation = repr;

	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_fill_oic_rep_payload_value_array(OCRepPayload *payload,
		const char *key, struct icl_state_list_s *list)
{
	int i, j, len, ret;
	bool *b_arr;
	double *d_arr;
	char **str_arr;
	int64_t *i_arr;
	OCByteString *byte_arr;
	struct OCRepPayload **state_arr;
	GList *node;

	RETV_IF(NULL == payload, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == key, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	if (NULL == list->list || 0 == g_list_length(list->list)) {
		DBG("list is empty");
		return IOTCON_ERROR_NONE;
	}

	len = calcDimTotal(list->dimensions);

	switch (list->type) {
	case OCREP_PROP_BOOL:
		b_arr = calloc(len, sizeof(bool));
		if (NULL == b_arr) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
		for (node = list->list, i = 0; node; node = node->next, i++)
			b_arr[i] = ((icl_basic_s*)node->data)->val.b;
		OCRepPayloadSetBoolArrayAsOwner(payload, key, b_arr, list->dimensions);
		break;
	case OCREP_PROP_INT:
		i_arr = calloc(len, sizeof(int64_t));
		if (NULL == i_arr) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
		for (node = list->list, i = 0; node; node = node->next, i++)
			i_arr[i] = ((icl_basic_s*)node->data)->val.b;
		OCRepPayloadSetIntArrayAsOwner(payload, key, i_arr, list->dimensions);
		break;
	case OCREP_PROP_DOUBLE:
		d_arr = calloc(len, sizeof(double));
		if (NULL == d_arr) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
		for (node = list->list, i = 0; node; node = node->next, i++)
			d_arr[i] = ((icl_basic_s*)node->data)->val.d;
		OCRepPayloadSetDoubleArrayAsOwner(payload, key, d_arr, list->dimensions);
		break;
	case OCREP_PROP_STRING:
		str_arr = calloc(len, sizeof(char *));
		if (NULL == str_arr) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
		for (node = list->list, i = 0; node; node = node->next, i++) {
			str_arr[i] = ((icl_basic_s*)node->data)->val.s;
			((icl_basic_s*)node->data)->val.s = NULL;
		}
		OCRepPayloadSetStringArrayAsOwner(payload, key, str_arr, list->dimensions);
		break;
	case OCREP_PROP_BYTE_STRING:
		byte_arr = calloc(len, sizeof(OCByteString));
		if (NULL == byte_arr) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
		for (node = list->list, i = 0; node; node = node->next, i++) {
			byte_arr[i].bytes = ((icl_val_byte_str_s*)node->data)->s;
			byte_arr[i].len = ((icl_val_byte_str_s*)node->data)->len;
		}
		OCRepPayloadSetByteStringArray(payload, key, byte_arr, list->dimensions);
		break;
	case OCREP_PROP_OBJECT:
		state_arr = calloc(len, sizeof(struct OCRepPayload *));
		if (NULL == state_arr) {
			ERR("calloc() Fail(%d)", errno);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}
		for (node = list->list, i = 0; node; node = node->next, i++) {
			state_arr[i] = OCRepPayloadCreate();
			if (NULL == state_arr[i]) {
				ERR("OCRepPayloadCreate() Fail");
				free(state_arr);
				return ret;
			}
			ret = _icl_ioty_fill_oic_rep_payload_value(state_arr[i],
					((icl_val_state_s*)node->data)->state);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_fill_oic_rep_payload_value() Fail(%d)", ret);
				for (j = 0; j <= i; j++)
					OCRepPayloadDestroy(state_arr[j]);
				free(state_arr);
				return ret;
			}
		}
		OCRepPayloadSetPropObjectArrayAsOwner(payload, key, state_arr, list->dimensions);
		break;
	case OCREP_PROP_ARRAY:
	case OCREP_PROP_NULL:
	default:
		ERR("Invalid Type(%d)", list->type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


static int _icl_ioty_state_list_to_value_list(iotcon_list_h list,
		struct icl_state_list_s *value_list, int depth)
{
	int ret;
	GList *c;

	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == value_list, IOTCON_ERROR_INVALID_PARAMETER);

	value_list->dimensions[depth] = g_list_length(list->list);

	switch (list->type) {
	case IOTCON_TYPE_BOOL:
		value_list->type = OCREP_PROP_BOOL;
		for (c = list->list; c; c = c->next)
			value_list->list = g_list_append(value_list->list, c->data);
		break;
	case IOTCON_TYPE_INT:
		value_list->type = OCREP_PROP_INT;
		for (c = list->list; c; c = c->next)
			value_list->list = g_list_append(value_list->list, c->data);
		break;
	case IOTCON_TYPE_DOUBLE:
		value_list->type = OCREP_PROP_DOUBLE;
		for (c = list->list; c; c = c->next)
			value_list->list = g_list_append(value_list->list, c->data);
		break;
	case IOTCON_TYPE_STR:
		value_list->type = OCREP_PROP_STRING;
		for (c = list->list; c; c = c->next)
			value_list->list = g_list_append(value_list->list, c->data);
		break;
	case IOTCON_TYPE_BYTE_STR:
		value_list->type = OCREP_PROP_BYTE_STRING;
		for (c = list->list; c; c = c->next)
			value_list->list = g_list_append(value_list->list, c->data);
		break;
	case IOTCON_TYPE_STATE:
		value_list->type = OCREP_PROP_OBJECT;
		for (c = list->list; c; c = c->next)
			value_list->list = g_list_append(value_list->list, c->data);
		break;
	case IOTCON_TYPE_LIST:
		for (c = list->list; c; c = c->next) {
			ret = _icl_ioty_state_list_to_value_list(c->data, value_list, depth + 1);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_state_list_to_value_list() Fail(%d)", ret);
				return ret;
			}
		}
		break;
	case IOTCON_TYPE_NULL:
	default:
		ERR("Invalid type (%d)", list->type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_fill_oic_rep_payload_value(OCRepPayload *payload,
		iotcon_state_h state)
{
	FN_CALL;
	int ret;
	GHashTableIter iter;
	gpointer key, value;
	OCRepPayload *repr_payload;
	OCByteString byte_string;
	struct icl_value_s *state_value;
	struct icl_state_list_s value_list = {0};

	RETV_IF(NULL == state, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == payload, IOTCON_ERROR_INVALID_PARAMETER);

	g_hash_table_iter_init(&iter, state->hash_table);
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		state_value = (struct icl_value_s *)value;
		if (NULL == state_value) {
			ERR("state_value(%s) is NULL", key);
			continue;
		}

		switch (state_value->type) {
		case IOTCON_TYPE_BOOL:
			OCRepPayloadSetPropBool(payload, key, ((icl_basic_s*)state_value)->val.b);
			break;
		case IOTCON_TYPE_INT:
			OCRepPayloadSetPropInt(payload, key, ((icl_basic_s*)state_value)->val.i);
			break;
		case IOTCON_TYPE_DOUBLE:
			OCRepPayloadSetPropDouble(payload, key, ((icl_basic_s*)state_value)->val.d);
			break;
		case IOTCON_TYPE_STR:
			OCRepPayloadSetPropString(payload, key, ((icl_basic_s*)state_value)->val.s);
			break;
		case IOTCON_TYPE_NULL:
			OCRepPayloadSetNull(payload, key);
			break;
		case IOTCON_TYPE_BYTE_STR:
			byte_string.bytes = ((icl_val_byte_str_s*)state_value)->s;
			byte_string.len = ((icl_val_byte_str_s*)state_value)->len;
			OCRepPayloadSetPropByteString(payload, key, byte_string);
			break;
		case IOTCON_TYPE_LIST:
			ret = _icl_ioty_state_list_to_value_list(((icl_val_list_s*)state_value)->list,
					&value_list, 0);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_state_list_to_value_list() Fail(%d)", ret);
				return ret;
			}
			ret = _icl_ioty_fill_oic_rep_payload_value_array(payload, key, &value_list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_fill_oic_rep_payload_value_array() Fail(%d)", ret);
				g_list_free(value_list.list);
				return ret;
			}
			g_list_free(value_list.list);
			value_list.list = NULL;
			break;
		case IOTCON_TYPE_STATE:
			repr_payload = OCRepPayloadCreate();
			ret = _icl_ioty_fill_oic_rep_payload_value(repr_payload,
					((icl_val_state_s*)state_value)->state);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_fill_oic_rep_payload_value() Fail(%d)", ret);
				OCRepPayloadDestroy(repr_payload);
				return ret;
			}
			OCRepPayloadSetPropObjectAsOwner(payload, key, repr_payload);
			break;
		case IOTCON_TYPE_NONE:
		default:
			ERR("Invalid Type(%d)", state_value->type);
			return IOTCON_ERROR_INVALID_PARAMETER;
		}
	}
	return IOTCON_ERROR_NONE;
}

int icl_ioty_convert_representation(iotcon_representation_h repr,
		OCPayload **payload)
{
	FN_CALL;
	int ret;
	GList *c;
	OCRepPayload *repr_payload, *cur;

	RETV_IF(NULL == payload, IOTCON_ERROR_INVALID_PARAMETER);

	repr_payload = OCRepPayloadCreate();

	if (NULL == repr) {
		*payload = (OCPayload *)repr_payload;
		return IOTCON_ERROR_NONE;
	}

	/* uri path */
	if (repr->uri_path)
		OCRepPayloadSetUri(repr_payload, repr->uri_path);

	/* interfaces */
	if (repr->interfaces) {
		for (c = repr->interfaces->iface_list; c; c = c->next)
			OCRepPayloadAddInterface(repr_payload, c->data);
	} else {
		OCRepPayloadAddInterface(repr_payload, IOTCON_INTERFACE_DEFAULT);
	}

	/* resource types */
	if (repr->res_types) {
		for (c = repr->res_types->type_list; c; c = c->next)
			OCRepPayloadAddResourceType(repr_payload, c->data);
	}

	/* state */
	if (repr->state) {
		ret = _icl_ioty_fill_oic_rep_payload_value(repr_payload, repr->state);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_ioty_fill_oic_rep_payload_value() Fail(%d)", ret);
			OCRepPayloadDestroy(repr_payload);
			return ret;
		}
	}

	/* children */
	cur = repr_payload;
	for (c = repr->children; c; c = c->next) {
		OCPayload *child = NULL;
		ret = icl_ioty_convert_representation(c->data, &child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_ioty_convert_representation() Fail(%d)", ret);
			OCRepPayloadDestroy(repr_payload);
			return IOTCON_ERROR_IOTIVITY;
		}
		cur->next = (OCRepPayload *)child;
		cur = cur->next;
	}

	*payload = (OCPayload *)repr_payload;
	return IOTCON_ERROR_NONE;
}

int icl_ioty_parse_oic_header_option(OCHeaderOption *option, int option_size,
		iotcon_options_h *options)
{
	int i, ret;
	iotcon_options_h op;

	RETV_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER);

	if (option_size <= 0) {
		*options = NULL;
		return IOTCON_ERROR_NONE;
	}

	ret = iotcon_options_create(&op);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_options_create() Fail(%d)", ret);
		return ret;
	}

	for (i = 0; i < option_size; i++)
		iotcon_options_add(op, option[i].optionID, (char *)option[i].optionData);

	*options = op;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_convert_header_options(iotcon_options_h options,
		OCHeaderOption dest[], int dest_size)
{
	int i = 0, src_size;
	GHashTableIter iter;
	gpointer option_id, option_data;

	RETV_IF(NULL == options, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == options->hash, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	src_size = g_hash_table_size(options->hash);

	if (dest_size < src_size) {
		ERR("Exceed Size(%d)", src_size);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	g_hash_table_iter_init(&iter, options->hash);
	while (g_hash_table_iter_next(&iter, &option_id, &option_data)) {
		dest[i].protocolID = OC_COAP_ID;
		dest[i].optionID = GPOINTER_TO_INT(option_id);
		dest[i].optionLength = strlen(option_data) + 1;
		memcpy(dest[i].optionData, option_data, dest[i].optionLength);
		i++;
	}
	return IOTCON_ERROR_NONE;
}

