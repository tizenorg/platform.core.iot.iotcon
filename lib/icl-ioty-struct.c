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
#include <octypes.h>
#include <ocstack.h>
#include <ocpayload.h>
#include <ocrandom.h>

#include "iotcon.h"
#include "ic-utils.h"
#include "ic-ioty-utils.h"
#include "icl.h"
#include "icl-device.h"
#include "icl-remote-resource.h"
#include "icl-presence.h"
#include "icl-ioty.h"
#include "icl-struct.h"
#include "icl-ioty-struct.h"

static int _icl_ioty_parse_repr_payload_value(OCRepPayloadValue *val,
		iotcon_state_h *state);

static void _icl_ioty_free_resource_list(iotcon_remote_resource_h *resource_list,
		int resource_count)
{
	int i;

	RET_IF(NULL == resource_list);

	for (i = 0; i < resource_count; i++)
		iotcon_remote_resource_destroy(resource_list[i]);

	free(resource_list);
}

int icl_ioty_parse_find_payload(OCDevAddr *dev_addr, OCDiscoveryPayload *payload,
		iotcon_remote_resource_h **resource_list, int *resource_count)
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
		ERR("calloc() Fail");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	for (i = 0; res_payload; i++, res_payload = res_payload->next) {
		int ifaces, port, properties;
		iotcon_interface_e iface;
		iotcon_connectivity_type_e conn_type;
		iotcon_resource_types_h types;
		char host_addr[PATH_MAX] = {0};
		char device_id[UUID_STRING_SIZE] = {0};
		OCStringLL *node;
		OCRandomUuidResult random_res;

		/* uri path */
		if (NULL == res_payload->uri) {
			ERR("res_payload uri is NULL");
			_icl_ioty_free_resource_list(res_list, res_count);
			return IOTCON_ERROR_IOTIVITY;
		}

		/* device id */
		random_res = OCConvertUuidToString(payload->sid, device_id);
		if (RAND_UUID_OK != random_res) {
			ERR("OCConvertUuidToString() Fail(%d)", random_res);
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
		ifaces = 0;
		node = res_payload->interfaces;
		if (NULL == node) {
			ERR("res_payload interfaces is NULL");
			iotcon_resource_types_destroy(types);
			continue;
		}
		for (; node; node = node->next) {
			ret = ic_utils_convert_interface_string(node->value, &iface);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("ic_utils_convert_interface_string() Fail(%d)", ret);
				iotcon_resource_types_destroy(types);
				_icl_ioty_free_resource_list(res_list, res_count);
				return ret;
			}
			ifaces |= iface;
		}

		/* Resource Properties */
		properties = ic_ioty_utils_parse_oic_properties(res_payload->bitmap);

		/* port */
		port = (res_payload->port) ? res_payload->port : dev_addr->port;

		/* connectivity type */
		conn_type = ic_ioty_utils_parse_oic_transport(dev_addr->adapter, dev_addr->flags);

		/* host address */
		switch (conn_type) {
		case IOTCON_CONNECTIVITY_IPV6:
			snprintf(host_addr, sizeof(host_addr), "[%s]:%d", dev_addr->addr, port);
			break;
		case IOTCON_CONNECTIVITY_IPV4:
			snprintf(host_addr, sizeof(host_addr), "%s:%d", dev_addr->addr, port);
			break;
		case IOTCON_CONNECTIVITY_BT_EDR:
		default:
			snprintf(host_addr, sizeof(host_addr), "%s", dev_addr->addr);
		}

		ret = iotcon_remote_resource_create(host_addr, conn_type, res_payload->uri,
				properties,	types, ifaces, &(res_list[i]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("iotcon_remote_resource_create() Fail(%d)", ret);
			iotcon_resource_types_destroy(types);
			_icl_ioty_free_resource_list(res_list, res_count);
			return ret;
		}

		res_list[i]->device_id = strdup(device_id);
		if (NULL == res_list[i]->device_id) {
			ERR("strdup(device_id) Fail(%d)", errno);
			_icl_ioty_free_resource_list(res_list, res_count);
			return IOTCON_ERROR_OUT_OF_MEMORY;
		}

		iotcon_resource_types_destroy(types);
	}

	*resource_list = res_list;
	*resource_count = res_count;
	return IOTCON_ERROR_NONE;

}

int icl_ioty_parse_device_info_payload(OCDevicePayload *payload,
		iotcon_device_info_h *device_info)
{
	char device_id[UUID_STRING_SIZE] = {0};
	struct icl_device_info *info = NULL;
	OCRandomUuidResult random_res;

	random_res = OCConvertUuidToString(payload->sid, device_id);
	if (RAND_UUID_OK != random_res) {
		ERR("OCConvertUuidToString() Fail(%d)", random_res);
		return IOTCON_ERROR_IOTIVITY;
	}

	info = calloc(1, sizeof(struct icl_device_info));
	if (NULL == info) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	info->device_name = ic_utils_strdup(payload->deviceName);
	info->spec_ver = ic_utils_strdup(payload->specVersion);
	info->device_id = strdup(device_id);
	info->data_model_ver = ic_utils_strdup(payload->dataModelVersion);

	*device_info = info;
	return IOTCON_ERROR_NONE;
}

int icl_ioty_parse_platform_info_payload(OCPlatformPayload *payload,
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
	info->manuf_url = ic_utils_strdup(payload->info.manufacturerUrl);
	info->model_number = ic_utils_strdup(payload->info.modelNumber);
	info->date_of_manuf = ic_utils_strdup(payload->info.dateOfManufacture);
	info->platform_ver = ic_utils_strdup(payload->info.platformVersion);
	info->os_ver = ic_utils_strdup(payload->info.operatingSystemVersion);
	info->hardware_ver = ic_utils_strdup(payload->info.hardwareVersion);
	info->firmware_ver = ic_utils_strdup(payload->info.firmwareVersion);
	info->support_url = ic_utils_strdup(payload->info.supportUrl);
	info->system_time = ic_utils_strdup(payload->info.systemTime);

	*platform_info = info;

	return IOTCON_ERROR_NONE;
}

int icl_ioty_parse_presence(OCDevAddr *dev_addr, OCPresencePayload *payload,
		OCStackResult result, iotcon_presence_response_h *presence_response)
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

	ret = ic_ioty_utils_parse_oic_dev_address(dev_addr, &(resp->host_address),
			(int*)&(resp->connectivity_type));
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_utils_parse_oic_dev_address() Fail(%d)", ret);
		icl_destroy_presence_response(resp);
		return ret;
	}

	switch (result) {
	case OC_STACK_OK:
		resp->result = IOTCON_PRESENCE_OK;
		ret = ic_ioty_utils_convert_trigger(payload->trigger, &resp->trigger);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_ioty_utils_convert_trigger() Fail(%d)", ret);
			icl_destroy_presence_response(resp);
			return ret;
		}
		break;
	case OC_STACK_PRESENCE_STOPPED:
		resp->result = IOTCON_PRESENCE_STOPPED;
		break;
	case OC_STACK_PRESENCE_TIMEOUT:
		resp->result = IOTCON_PRESENCE_TIMEOUT;
		break;
	case OC_STACK_ERROR:
	default:
		DBG("Presence error(%d)", resp->result);
		resp->result = IOTCON_ERROR_IOTIVITY;
	}
	if (payload->resourceType)
		resp->resource_type = strdup(payload->resourceType);

	*presence_response = resp;

	return IOTCON_ERROR_NONE;
}

static int _icl_ioty_parse_repr_payload_value_array_attr(OCRepPayloadValueArray *arr,
		int len, int index, iotcon_list_h *list)
{
	int i, ret;
	iotcon_list_h l = NULL;

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
			ret = _icl_ioty_parse_repr_payload_value(arr->objArray[index + i]->values, &state);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_parse_repr_payload_value");
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

static int _icl_ioty_parse_repr_payload_value_array(OCRepPayloadValueArray *arr,
		int depth, int len, int index, iotcon_list_h *list)
{
	int i, ret, next_len;
	iotcon_list_h l;

	RETV_IF(NULL == arr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == list, IOTCON_ERROR_INVALID_PARAMETER);

	if ((MAX_REP_ARRAY_DEPTH - 1) == depth || 0 == arr->dimensions[depth + 1]) {
		ret = _icl_ioty_parse_repr_payload_value_array_attr(arr, len, index, &l);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_ioty_parse_repr_payload_value_array_attr() Fail(%d)", ret);
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
		ret = _icl_ioty_parse_repr_payload_value_array(arr, depth + 1, next_len, index, &l_child);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icl_ioty_parse_repr_payload_value_array() Fail(%d)", ret);
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

static int _icl_ioty_parse_repr_payload_value(OCRepPayloadValue *val,
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
			ret = iotcon_state_add_byte_str(s, val->name, val->ocByteStr.bytes, val->ocByteStr.len);
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
			ret = _icl_ioty_parse_repr_payload_value_array(&val->arr, 0, total_len, 0, &list);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_parse_repr_payload_value_array() Fail(%d)", ret);
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
			ret = _icl_ioty_parse_repr_payload_value(val->obj->values, &s_obj);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icl_ioty_parse_repr_payload_value() Fail(%d)", ret);
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

int icl_ioty_parse_representation_payload(OCRepPayload *payload, bool is_parent,
		iotcon_representation_h *representation)
{
	int ret, ifaces = 0;;
	OCStringLL *node;
	OCRepPayload *child_node;
	iotcon_interface_e iface;
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

	/* resource interfaces */
	node = payload->interfaces;
	while (node) {
		ret = ic_utils_convert_interface_string(node->value, &iface);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_utils_convert_interface_string() Fail(%d)", ret);
			iotcon_representation_destroy(repr);
			return ret;
		}
		ifaces |= iface;
		node = node->next;
	}
	ret = iotcon_representation_set_resource_interfaces(repr, ifaces);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("iotcon_representation_set_resource_types() Fail(%d)", ret);
		iotcon_representation_destroy(repr);
		return ret;
	}

	/* state */
	if (payload->values) {
		ret = _icl_ioty_parse_repr_payload_value(payload->values, &state);
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
	if (payload->next) {
		child_node = payload->next;
		while (is_parent && child_node) {
			/* generate recursively */
			iotcon_representation_h repr_child;

			ret = icl_ioty_parse_representation_payload(child_node, false, &repr_child);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_ioty_parse_representation_payload() Fail(%d)", ret);
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
	}

	*representation = repr;

	return IOTCON_ERROR_NONE;
}


