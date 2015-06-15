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
#include <stdbool.h>
#include <stdint.h>
#include <glib.h>
#include <OCApi.h>
#include <OCPlatform.h>

extern "C" {
#include "iotcon-struct.h"
#include "icl.h"
#include "icl-utils.h"
#include "icl-client.h"
#include "icl-request.h"
#include "icl-response.h"
#include "icl-resource-types.h"
#include "icl-repr.h"
#include "icl-ioty-repr.h"
#include "icl-ioty.h"
}

#define IC_COAP "coap://"
#define IC_UNICAST_RESOURCE_DISCOVERY "/oc/core"
#define IC_MULTICAST_RESOURCE_DISCOVERY "/oc/core"
#define IC_DEVICE_DISCOVERY "/oc/core/d"

using namespace std;
using namespace OC;
using namespace OCPlatform;

struct resource_handle {
	OCResource::Ptr ocResource;
};

namespace icIotivityHandler {

	class presenceObject
	{
	private:
		iotcon_presence_cb presence_handler;
		void *cb_data;

	public:
		presenceObject(iotcon_presence_cb user_cb, void *user_data)
		{
			presence_handler = user_cb;
			cb_data = user_data;
		}

		void presenceHandler(OCStackResult result, const unsigned int nonce,
				const string& hostAddress)
		{
			int res;

			switch (result) {
			case OC_STACK_OK:
				res = IOTCON_PRESENCE_OK;
				break;
			case OC_STACK_PRESENCE_STOPPED:
				res = IOTCON_PRESENCE_STOPPED;
				break;
			case OC_STACK_PRESENCE_TIMEOUT:
				res = IOTCON_PRESENCE_TIMEOUT;
				break;
			case OC_STACK_ERROR:
			default:
				ERR("subscribePresence() Fail(%d)", result);
				res = IOTCON_PRESENCE_ERROR;
			}

			if (presence_handler)
				presence_handler(res, nonce, hostAddress.c_str(), cb_data);
		}
	};

	class findObject
	{
	private:
		iotcon_found_resource_cb found_resource;
		void *cb_data;

	public:
		findObject(iotcon_found_resource_cb user_cb, void *user_data)
		{
			found_resource = user_cb;
			cb_data = user_data;
		}

		void foundResource(shared_ptr<OCResource> resource)
		{
			struct ic_remote_resource resource_s = {0};
			resource_s.types = NULL;
			string resource_host;

			vector<string> resource_types = resource->getResourceTypes();
			if (0 < resource_types.size()) {
				resource_s.types = iotcon_resource_types_new();
				if (NULL == resource_s.types) {
					ERR("iotcon_resource_types_new() Fail");
					return;
				}

				for (string &resource_type : resource_types)
					iotcon_resource_types_insert(resource_s.types, resource_type.c_str());
			}

			resource_s.uri = ic_utils_strdup(resource->uri().c_str());

			resource_host = resource->host();
			resource_host.erase(0, strlen(IC_COAP));
			resource_s.host = ic_utils_strdup(resource_host.c_str());

			resource_s.is_observable = resource->isObservable();

			vector<string> resource_interfaces = resource->getResourceInterfaces();
			for (string &resource_interface : resource_interfaces) {
				if (IC_STR_EQUAL == resource_interface.compare(DEFAULT_INTERFACE))
					resource_s.ifaces |= IOTCON_INTERFACE_DEFAULT;

				if (IC_STR_EQUAL == resource_interface.compare(BATCH_INTERFACE))
					resource_s.ifaces |= IOTCON_INTERFACE_BATCH;

				if (IC_STR_EQUAL == resource_interface.compare(LINK_INTERFACE))
					resource_s.ifaces |= IOTCON_INTERFACE_LINK;

				if (IC_STR_EQUAL == resource_interface.compare(GROUP_INTERFACE))
					resource_s.ifaces |= IOTCON_INTERFACE_GROUP;
			}

			if (found_resource)
				found_resource(&resource_s, cb_data);

			free(resource_s.uri);
			free(resource_s.host);
			iotcon_resource_types_free(resource_s.types);
		}
	};

	class getObject
	{
	private:
		iotcon_on_cru_cb on_get;
		void *cb_data;

	public:
		getObject(iotcon_on_cru_cb user_cb, void *user_data)
		{
			on_get = user_cb;
			cb_data = user_data;
		}

		void onGet(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode)
		{
			FN_CALL;
			int res;
			iotcon_options_h options;
			iotcon_repr_h repr = NULL;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else {
				ERR("get() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_get)
				on_get(options, repr, res, cb_data);

			iotcon_repr_free(repr);
			iotcon_options_free(options);
		}
	};

	class putObject
	{
	private:
		iotcon_on_cru_cb on_put;
		void *cb_data;

	public:
		putObject(iotcon_on_cru_cb user_cb, void *user_data)
		{
			on_put = user_cb;
			cb_data = user_data;
		}

		void onPut(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode)
		{
			FN_CALL;
			int res;
			iotcon_options_h options;
			iotcon_repr_h repr = NULL;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else {
				ERR("put() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_put)
				on_put(options, repr, res, cb_data);

			iotcon_repr_free(repr);
			iotcon_options_free(options);
		}
	};

	class postObject
	{
	private:
		iotcon_on_cru_cb on_post;
		void *cb_data;

	public:
		postObject(iotcon_on_cru_cb user_cb, void *user_data)
		{
			on_post = user_cb;
			cb_data = user_data;
		}

		void onPost(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode)
		{
			FN_CALL;
			int res;
			iotcon_options_h options;
			iotcon_repr_h repr = NULL;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else if (OC_STACK_RESOURCE_CREATED == eCode) {
				res = IOTCON_RESPONSE_RESULT_RESOURCE_CREATED;
			} else {
				ERR("post() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_post)
				on_post(options, repr, res, cb_data);

			iotcon_repr_free(repr);
			iotcon_options_free(options);
		}
	};

	class deleteObject
	{
	private:
		iotcon_on_delete_cb on_delete;
		void *cb_data;

	public:
		deleteObject(iotcon_on_delete_cb user_cb, void *user_data)
		{
			on_delete = user_cb;
			cb_data = user_data;
		}

		void onDelete(const HeaderOptions& headerOptions, const int eCode)
		{
			int res;
			iotcon_options_h options;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else if (OC_STACK_RESOURCE_DELETED == eCode) {
				res = IOTCON_RESPONSE_RESULT_RESOURCE_DELETED;
			} else {
				ERR("deleteResource() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			if (on_delete)
				on_delete(options, res, cb_data);

			iotcon_options_free(options);
		}
	};

	class observeObject
	{
	private:
		iotcon_on_observe_cb on_observe;
		void *cb_data;

	public:
		observeObject(iotcon_on_observe_cb user_cb, void *user_data)
		{
			on_observe = user_cb;
			cb_data = user_data;
		}

		void onObserve(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode, const int sequenceNumber)
		{
			FN_CALL;
			int res;
			iotcon_options_h options;
			iotcon_repr_h repr = NULL;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else {
				ERR("observe() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_observe)
				on_observe(options, repr, res, sequenceNumber, cb_data);

			iotcon_repr_free(repr);
			iotcon_options_free(options);
		}
	};

	class deviceObject
	{
	private:
		iotcon_device_info_cb found_cb;
		void *cb_data;

	public:
		deviceObject(iotcon_device_info_cb user_cb, void *user_data)
		{
			found_cb = user_cb;
			cb_data = user_data;
		}

		void receivedDeviceInfo(const OCRepresentation& ocRep)
		{
			iotcon_device_info_s info = {0};
			string readbuf;

			if (ocRep.getValue("ct", readbuf))
				info.content_type = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("mndt", readbuf))
				info.date_of_manufacture = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("dn", readbuf))
				info.name = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("di", readbuf))
				info.uuid = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("mnfv", readbuf))
				info.firmware_ver = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("hn", readbuf))
				info.host_name = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("mnmn", readbuf))
				info.manuf_name = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("mnml", readbuf))
				info.manuf_url = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("mnmo", readbuf))
				info.model_number = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("mnpv", readbuf))
				info.platform_ver = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("mnsl", readbuf))
				info.support_url = ic_utils_strdup(readbuf.c_str());
			if (ocRep.getValue("icv", readbuf))
				info.version = ic_utils_strdup(readbuf.c_str());

			if (found_cb)
				found_cb(info, cb_data);

			free(info.name);
			free(info.host_name);
			free(info.uuid);
			free(info.content_type);
			free(info.version);
			free(info.manuf_name);
			free(info.manuf_url);
			free(info.model_number);
			free(info.date_of_manufacture);
			free(info.platform_ver);
			free(info.firmware_ver);
			free(info.support_url);
		}
	};
}

extern "C" void ic_ioty_config(const char *addr, unsigned short port)
{
	PlatformConfig cfg {
		ServiceType::InProc,
			ModeType::Both,
			string(addr),
			port,
			QualityOfService::HighQos
	};
	Configure(cfg);
	DBG("Created a platform");
}

static OCEntityHandlerResult _ic_ioty_request_handler(
		shared_ptr<OCResourceRequest> request)
{
	FN_CALL;
	const char *request_type = NULL;
	HeaderOptions headerOptions;
	QueryParamsMap queryParams;
	iotcon_resource_h temp_res = NULL;
	struct ic_resource_request request_s = {0};

	temp_res = ic_get_resource_handler_data(request->getResourceHandle());
	if (NULL == temp_res) {
		ERR("No Resource Handler");
		return OC_EH_ERROR;
	}

	queryParams = request->getQueryParameters();
	if (0 < queryParams.size()) {
		request_s.query = iotcon_query_new();
		if (NULL == request_s.query) {
			ERR("iotcon_query_new() Fail");
			return OC_EH_ERROR;
		}

		for (auto it : queryParams) {
			DBG("key = %s value = %s", it.first.c_str(), it.second.c_str());
			iotcon_query_insert(request_s.query, it.first.c_str(), it.second.c_str());
		}
	}

	headerOptions = request->getHeaderOptions();
	if (0 < headerOptions.size()) {
		request_s.header_options = iotcon_options_new();
		if (NULL == request_s.header_options) {
			ERR("iotcon_options_new() Fail");
			if (request_s.query)
				iotcon_query_free(request_s.query);
			return OC_EH_ERROR;
		}

		for (auto it : headerOptions) {
			DBG("OptionID = %d, OptionData = %s",
					it.getOptionID(), it.getOptionData().c_str());
			iotcon_options_insert(request_s.header_options, it.getOptionID(),
					it.getOptionData().c_str());
		}
	}

	OCRepresentation ocRep = request->getResourceRepresentation();
	if (0 < ocRep.numberOfAttributes()) {
		DBG("numberOfAttributes : %d", ocRep.numberOfAttributes());
		request_s.repr = ic_ioty_repr_generate_repr(ocRep);
		if (NULL == request_s.repr) {
			ERR("request_s.repr is NULL");
			if (request_s.header_options)
				iotcon_options_free(request_s.header_options);
			if (request_s.query)
				iotcon_query_free(request_s.query);
			return OC_EH_ERROR;
		}
	}

	if (RequestFlag & request->getRequestHandlerFlag()) {
		request_type = ic_utils_strdup(request->getRequestType().c_str());
		if (NULL == request_type) {
			ERR("request_type is NULL");
			if (request_s.repr)
				iotcon_repr_free(request_s.repr);
			if (request_s.header_options)
				iotcon_options_free(request_s.header_options);
			if (request_s.query)
				iotcon_query_free(request_s.query);
			return OC_EH_ERROR;
		}

		if (IC_STR_EQUAL == strcmp("GET", request_type))
			request_s.types = IOTCON_REQUEST_GET;
		else if (IC_STR_EQUAL == strcmp("PUT", request_type))
			request_s.types = IOTCON_REQUEST_PUT;
		else if (IC_STR_EQUAL == strcmp("POST", request_type))
			request_s.types = IOTCON_REQUEST_POST;
		else if (IC_STR_EQUAL == strcmp("DELETE", request_type))
			request_s.types = IOTCON_REQUEST_DELETE;
	}

	if (ObserverFlag & request->getRequestHandlerFlag())
		request_s.types |= IOTCON_REQUEST_OBSERVE;

	request_s.uri = ic_utils_strdup(request->getResourceUri().c_str());
	if (NULL == request_s.uri) {
		ERR("ic_utils_strdup() Fail");
		if (request_s.repr)
			iotcon_repr_free(request_s.repr);
		if (request_s.header_options)
			iotcon_options_free(request_s.header_options);
		if (request_s.query)
			iotcon_query_free(request_s.query);
		return OC_EH_ERROR;
	}

	request_s.request_handle = request->getRequestHandle();
	request_s.resource_handle = request->getResourceHandle();

	ObservationInfo observationInfo = request->getObservationInfo();
	request_s.observation_info.action = (iotcon_observe_action_e)observationInfo.action;
	request_s.observation_info.observer_id = observationInfo.obsId;
	DBG("obs_info.obsId=%d", observationInfo.obsId);

	/* call handler_cb */
	if (temp_res->cb)
		temp_res->cb(&request_s, temp_res->user_data);
	else
		WARN("temp_res->request_handler_cb is null");

	free(request_s.uri);

	/* To avoid unnecessary ERR log (repr could be NULL) */
	if (request_s.repr)
		iotcon_repr_free(request_s.repr);
	if (request_s.header_options)
		iotcon_options_free(request_s.header_options);
	if (request_s.query)
		iotcon_query_free(request_s.query);

	return OC_EH_OK;
}


extern "C" OCResourceHandle ic_ioty_register_res(const char *uri,
		iotcon_resource_types_h res_types, int ifaces, uint8_t properties)
{
	FN_CALL;
	unsigned int i;
	OCStackResult ret;
	string resUri;
	string resType;
	string resInterface;
	OCResourceHandle handle;

	resUri = uri;

	resType = ic_resource_types_get_nth_data(res_types, 0);

	if (IOTCON_INTERFACE_DEFAULT & ifaces) {
		resInterface = DEFAULT_INTERFACE;
		ifaces ^= IOTCON_INTERFACE_DEFAULT;
	} else if (IOTCON_INTERFACE_LINK & ifaces) {
		resInterface = LINK_INTERFACE;
		ifaces ^= IOTCON_INTERFACE_LINK;
	} else if (IOTCON_INTERFACE_BATCH & ifaces) {
		resInterface = BATCH_INTERFACE;
		ifaces ^= IOTCON_INTERFACE_BATCH;
	} else if (IOTCON_INTERFACE_GROUP & ifaces) {
		resInterface = GROUP_INTERFACE;
		ifaces ^= IOTCON_INTERFACE_GROUP;
	}

	ret = registerResource(handle, resUri, resType, resInterface,
			_ic_ioty_request_handler, properties);
	if (OC_STACK_OK != ret) {
		ERR("registerResource Fail(%d)", ret);
		return NULL;
	}
	for (i = 1; i < ic_resource_types_get_length(res_types); i++)
		ic_ioty_bind_type_to_res(handle, ic_resource_types_get_nth_data(res_types, i));

	if (IOTCON_INTERFACE_DEFAULT & ifaces)
		ic_ioty_bind_iface_to_res(handle, IOTCON_INTERFACE_DEFAULT);

	if (IOTCON_INTERFACE_LINK & ifaces)
		ic_ioty_bind_iface_to_res(handle, IOTCON_INTERFACE_LINK);

	if (IOTCON_INTERFACE_BATCH & ifaces)
		ic_ioty_bind_iface_to_res(handle, IOTCON_INTERFACE_BATCH);

	if (IOTCON_INTERFACE_GROUP & ifaces)
		ic_ioty_bind_iface_to_res(handle, IOTCON_INTERFACE_GROUP);

	return handle;
}


extern "C" int ic_ioty_unregister_res(iotcon_resource_h resource_handle)
{
	OCResourceHandle resourceHandle = resource_handle;

	OCStackResult result = unregisterResource(resourceHandle);
	if (OC_STACK_OK != result) {
		ERR("unregisterResource Fail(%d)", result);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_convert_interface_string(const char *src, iotcon_interface_e *dest)
{
	RETV_IF(NULL == src, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	string interface_str(src);

	if (IC_STR_EQUAL == DEFAULT_INTERFACE.compare(interface_str)) {
		*dest = IOTCON_INTERFACE_DEFAULT;
	} else if (IC_STR_EQUAL == LINK_INTERFACE.compare(interface_str)) {
		*dest = IOTCON_INTERFACE_LINK;
	} else if (IC_STR_EQUAL == BATCH_INTERFACE.compare(interface_str)) {
		*dest = IOTCON_INTERFACE_BATCH;
	} else if (IC_STR_EQUAL == GROUP_INTERFACE.compare(interface_str)) {
		*dest = IOTCON_INTERFACE_GROUP;
	} else {
		ERR("Invalid interface");
		*dest = IOTCON_INTERFACE_NONE;
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}

static int _ic_ioty_convert_interface_flag(iotcon_interface_e src, string &dest)
{
	switch (src) {
	case IOTCON_INTERFACE_GROUP:
		dest = GROUP_INTERFACE;
		break;
	case IOTCON_INTERFACE_BATCH:
		dest = BATCH_INTERFACE;
		break;
	case IOTCON_INTERFACE_LINK:
		dest = LINK_INTERFACE;
		break;
	case IOTCON_INTERFACE_DEFAULT:
		dest = DEFAULT_INTERFACE;
		break;
	case IOTCON_INTERFACE_NONE:
	default:
		ERR("Invalid interface");
		dest = "";
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_convert_interface_flag(iotcon_interface_e src, char **dest)
{
	FN_CALL;
	int ret;
	string iface_str;

	ret = _ic_ioty_convert_interface_flag(src, iface_str);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ic_ioty_convert_interface_flag() Fail(%d)", ret);
		*dest = NULL;
		return ret;
	}

	*dest = ic_utils_strdup(iface_str.c_str());

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_bind_iface_to_res(OCResourceHandle resourceHandle,
		iotcon_interface_e iface)
{
	int ret;
	OCStackResult ocRet;
	string resource_interface;

	ret = _ic_ioty_convert_interface_flag(iface, resource_interface);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_ic_ioty_convert_interface_flag(%d) Fail(%d)", iface, ret);
		return ret;
	}

	ocRet = bindInterfaceToResource(resourceHandle, resource_interface);
	if (OC_STACK_OK != ocRet) {
		ERR("bindInterfaceToResource() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int ic_ioty_bind_type_to_res(OCResourceHandle resource_handle,
		const char *resource_type)
{
	OCStackResult ret;
	OCResourceHandle resourceHandle = resource_handle;

	ret = bindTypeToResource(resourceHandle, resource_type);
	if (OC_STACK_OK != ret) {
		ERR("bindTypeToResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_bind_res(OCResourceHandle parent, OCResourceHandle child)
{
	OCStackResult ret;

	ret = bindResource(parent, child);
	if (OC_STACK_OK != ret) {
		ERR("bindResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_unbind_res(OCResourceHandle parent, OCResourceHandle child)
{
	OCStackResult ret;

	ret = unbindResource(parent, child);
	if (OC_STACK_OK != ret) {
		ERR("unbindResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_register_device_info(iotcon_device_info_s device_info)
{
	FN_CALL;
	OCStackResult ret;

	OCDeviceInfo deviceInfo = {0};
	deviceInfo.deviceName = device_info.name;
	deviceInfo.hostName = device_info.host_name;
	deviceInfo.deviceUUID = device_info.uuid;
	deviceInfo.contentType = device_info.content_type;
	deviceInfo.version = device_info.version;
	deviceInfo.manufacturerName = device_info.manuf_name;
	deviceInfo.manufacturerUrl = device_info.manuf_url;
	deviceInfo.modelNumber = device_info.model_number;
	deviceInfo.dateOfManufacture = device_info.date_of_manufacture;
	deviceInfo.platformVersion = device_info.platform_ver;
	deviceInfo.firmwareVersion = device_info.firmware_ver;
	deviceInfo.supportUrl = device_info.support_url;

	ret = registerDeviceInfo(deviceInfo);
	if (OC_STACK_OK != ret) {
		ERR("registerDeviceInfo() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int ic_ioty_get_device_info(const char *host_address,
		iotcon_device_info_cb cb, void *user_data)
{
	OCStackResult ret;
	string resHost = string(IC_COAP) + host_address + string(IC_DEVICE_DISCOVERY);

	shared_ptr<icIotivityHandler::deviceObject> object
		= make_shared<icIotivityHandler::deviceObject>(cb, user_data);
	FindDeviceCallback findDeviceCallback = bind(
			&icIotivityHandler::deviceObject::receivedDeviceInfo,
			object,
			placeholders::_1);

	ret = getDeviceInfo("", resHost, findDeviceCallback);
	if (OC_STACK_OK != ret) {
		ERR("getDeviceInfo() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int ic_ioty_notify_list_of_observers(OCResourceHandle resHandle,
		struct ic_notify_msg *msg, iotcon_observers_h observers)
{
	int ret;
	OCStackResult ocRet;
	ObservationIds obsIds;
	OCRepresentation ocRep;
	string iface;

	GList *node = g_list_first((GList *)observers);
	while (node) {
		int obs_id = GPOINTER_TO_UINT(node->data);
		obsIds.push_back(obs_id);

		node = node->next;
	}

	shared_ptr<OCResourceResponse> resourceResponse(new OCResourceResponse());

	if (msg) {
		resourceResponse->setErrorCode(msg->error_code);

		ret = ic_ioty_repr_parse(msg->repr, ocRep);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("ic_ioty_repr_parse() Fail(%d)", ret);
			return ret;
		}

		if (IOTCON_INTERFACE_NONE != msg->iface) {
			ret = _ic_ioty_convert_interface_flag(msg->iface, iface);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_ic_ioty_convert_interface_flag(%d) Fail(%d)", msg->iface, ret);
				return ret;
			}
		} else {
			iface = DEFAULT_INTERFACE;
		}

		resourceResponse->setResourceRepresentation(ocRep, iface);
	}

	ocRet = notifyListOfObservers(resHandle, obsIds, resourceResponse);
	if (OC_STACK_NO_OBSERVERS == ocRet) {
		WARN("No observers. Stop notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ocRet) {
		ERR("notifyListOfObservers() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int ic_ioty_notify_all(OCResourceHandle resHandle)
{
	OCStackResult ocRet;

	ocRet = notifyAllObservers(resHandle);
	if (OC_STACK_NO_OBSERVERS == ocRet) {
		WARN("No observers. Stop notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ocRet) {
		ERR("notifyAllObservers() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int ic_ioty_send_res_response_data(struct ic_resource_response *resp)
{
	FN_CALL;
	string iface;
	int ret;
	OCStackResult ocRet;
	OCRepresentation ocRep;

	ret = ic_ioty_repr_parse(resp->repr, ocRep);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_repr_parse() Fail(%d)", ret);
		return ret;
	}

	auto pResponse = make_shared<OCResourceResponse>();
	if (pResponse) {
		pResponse->setRequestHandle(resp->request_handle);
		pResponse->setResourceHandle(resp->resource_handle);
		pResponse->setErrorCode(resp->error_code);
		pResponse->setResponseResult((OCEntityHandlerResult)resp->result);

		if (IOTCON_INTERFACE_NONE != resp->iface) {
			ret = _ic_ioty_convert_interface_flag(resp->iface, iface);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_ic_ioty_convert_interface_flag(%d) Fail(%d)", resp->iface, ret);
				return ret;
			}
		} else {
			iface = DEFAULT_INTERFACE;
		}

		pResponse->setResourceRepresentation(ocRep, iface);

		ocRet = sendResponse(pResponse);
		if (OC_STACK_OK != ocRet) {
			ERR("sendResponse() Fail(%d)", ocRet);
			return IOTCON_ERROR_IOTIVITY;
		}
	}

	FN_END;

	return IOTCON_ERROR_NONE;

}

extern "C" const iotcon_presence_h ic_ioty_subscribe_presence(const char *host_address,
		const char *resource_type,
		iotcon_presence_cb cb,
		void *user_data)
{
	OCStackResult ret;
	iotcon_presence_h presence_handle = NULL;
	string host;

	shared_ptr<icIotivityHandler::presenceObject> object
		= make_shared<icIotivityHandler::presenceObject>(cb, user_data);
	SubscribeCallback subscribeCallback
		= bind(&icIotivityHandler::presenceObject::presenceHandler, object,
				placeholders::_1, placeholders::_2, placeholders::_3);

	host = string(IC_COAP) + string(host_address);
	ret = subscribePresence(presence_handle, host, resource_type, subscribeCallback);

	if (OC_STACK_OK != ret) {
		ERR("subscribePresence() Fail(%d)", ret);
		return NULL;
	}

	return presence_handle;
}

extern "C" int ic_ioty_unsubscribe_presence(iotcon_presence_h presence_handle)
{
	OCStackResult ret;

	ret = unsubscribePresence(presence_handle);
	if (OC_STACK_OK != ret) {
		ERR("unsubscribePresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_start_presence(unsigned int time_to_live)
{
	OCStackResult ret;

	ret = startPresence(time_to_live);
	if (OC_STACK_OK != ret) {
		ERR("startPresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_stop_presence()
{
	OCStackResult ret;

	ret = stopPresence();
	if (OC_STACK_OK != ret) {
		ERR("stopPresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_find_resource(const char *host_address, const char *resource_type,
		iotcon_found_resource_cb cb, void *user_data)
{
	OCStackResult ret;
	ostringstream resource_name;

	resource_name << IC_COAP;

	if (IC_STR_EQUAL == strcmp(IOTCON_MULTICAST_ADDRESS, host_address))
		resource_name << host_address << IC_MULTICAST_RESOURCE_DISCOVERY;
	else
		resource_name << host_address << IC_UNICAST_RESOURCE_DISCOVERY;

	if (resource_type)
		resource_name << "?rt=" << resource_type;

	shared_ptr<icIotivityHandler::findObject> object
		= make_shared<icIotivityHandler::findObject>(cb, user_data);
	FindCallback findCallback = bind(&icIotivityHandler::findObject::foundResource,
			object, placeholders::_1);

	ret = findResource("", resource_name.str(), findCallback);
	if (OC_STACK_OK != ret) {
		ERR("findResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

static int _ic_ioty_accumulate_options_vector(unsigned short id, const char *data,
		void *user_data)
{
	HeaderOptions *options = static_cast<HeaderOptions*>(user_data);
	HeaderOption::OCHeaderOption option(id, data);
	(*options).push_back(option);

	return IOTCON_FUNC_CONTINUE;
}


static int _ic_ioty_accumulate_res_types(const char *type, void *user_data)
{
	vector<string> *types = static_cast<vector<string>*>(user_data);
	(*types).push_back(type);

	return IOTCON_FUNC_CONTINUE;
}


static OCResource::Ptr _ic_ioty_create_oc_resource(iotcon_client_h resource)
{
	string host;
	string uri;
	vector<string> resource_types;
	vector<string> resource_ifs;

	HeaderOptions header_options;

	RETV_IF(NULL == resource, NULL);
	RETV_IF(NULL == resource->host, NULL);
	RETV_IF(NULL == resource->uri, NULL);
	RETV_IF(NULL == resource->types, NULL);

	host = string(IC_COAP) + string(resource->host);
	uri = resource->uri;

	iotcon_resource_types_foreach(resource->types, _ic_ioty_accumulate_res_types,
			(void *)&resource_types);

	if (IOTCON_INTERFACE_NONE == resource->ifaces) {
		resource_ifs.push_back(DEFAULT_INTERFACE);
	} else {
		if (IOTCON_INTERFACE_DEFAULT & resource->ifaces)
			resource_ifs.push_back(DEFAULT_INTERFACE);

		if (IOTCON_INTERFACE_LINK & resource->ifaces)
			resource_ifs.push_back(LINK_INTERFACE);

		if (IOTCON_INTERFACE_BATCH & resource->ifaces)
			resource_ifs.push_back(BATCH_INTERFACE);

		if (IOTCON_INTERFACE_GROUP & resource->ifaces)
			resource_ifs.push_back(GROUP_INTERFACE);
	}

	OCResource::Ptr ocResource = constructResourceObject(host, uri,
			resource->is_observable, resource_types, resource_ifs);

	if (resource->header_options) {
		iotcon_options_foreach(resource->header_options,
				_ic_ioty_accumulate_options_vector, (void *)&header_options);
		ocResource->setHeaderOptions(header_options);
	}

	return ocResource;
}


static int _ic_ioty_accumulate_query_map(const char *key, const char *value,
		void *user_data)
{
	QueryParamsMap *queryParams = static_cast<QueryParamsMap*>(user_data);
	string keyStr = key;
	string valueStr = value;
	(*queryParams)[keyStr] = valueStr;

	return IOTCON_FUNC_CONTINUE;
}


extern "C" int ic_ioty_get(iotcon_client_h resource, iotcon_query_h query,
		iotcon_on_cru_cb cb, void *user_data)
{
	FN_CALL;
	OCStackResult ret;
	OCResource::Ptr ocResource;
	QueryParamsMap queryParams;

	if (query)
		iotcon_query_foreach(query, _ic_ioty_accumulate_query_map, (void *)&queryParams);

	ocResource = _ic_ioty_create_oc_resource(resource);

	shared_ptr<icIotivityHandler::getObject> object
		= make_shared<icIotivityHandler::getObject>(cb, user_data);
	GetCallback getCallback = bind(&icIotivityHandler::getObject::onGet, object,
			placeholders::_1, placeholders::_2, placeholders::_3);
	ret = ocResource->get(queryParams, getCallback);
	if (OC_STACK_OK != ret) {
		ERR("get() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int ic_ioty_put(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult result;
	OCResource::Ptr ocResource;
	OCRepresentation ocRep;
	QueryParamsMap queryParams;

	if (query)
		iotcon_query_foreach(query, _ic_ioty_accumulate_query_map, (void *)&queryParams);

	ret = ic_ioty_repr_parse(repr, ocRep);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_repr_parse() Fail(%d)", ret);
		return ret;
	}

	ocResource = _ic_ioty_create_oc_resource(resource);

	shared_ptr<icIotivityHandler::putObject> object
		= make_shared<icIotivityHandler::putObject>(cb, user_data);
	PutCallback putCallback = bind(&icIotivityHandler::putObject::onPut, object,
			placeholders::_1, placeholders::_2, placeholders::_3);

	result = ocResource->put(ocRep, queryParams, putCallback);
	if (OC_STACK_OK != result) {
		ERR("put() Fail(%d)", result);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_post(iotcon_client_h resource, iotcon_repr_h repr,
		iotcon_query_h query, iotcon_on_cru_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	OCStackResult ocRet;
	QueryParamsMap queryParams;
	OCRepresentation ocRep;
	OCResource::Ptr ocResource;

	if (query)
		iotcon_query_foreach(query, _ic_ioty_accumulate_query_map, (void *)&queryParams);

	ret = ic_ioty_repr_parse(repr, ocRep);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_ioty_repr_parse() Fail(%d)", ret);
		return ret;
	}

	ocResource = _ic_ioty_create_oc_resource(resource);

	shared_ptr<icIotivityHandler::postObject> object
		= make_shared<icIotivityHandler::postObject>(cb, user_data);
	PostCallback postCallback = bind(&icIotivityHandler::postObject::onPost, object,
			placeholders::_1, placeholders::_2, placeholders::_3);

	ocRet = ocResource->post(ocRep, queryParams, postCallback);
	if (OC_STACK_OK != ocRet) {
		ERR("post() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_delete_res(iotcon_client_h resource,
		iotcon_on_delete_cb cb, void *user_data)
{
	FN_CALL;
	OCStackResult ret;
	OCResource::Ptr ocResource;

	ocResource = _ic_ioty_create_oc_resource(resource);

	shared_ptr<icIotivityHandler::deleteObject> object
		= make_shared<icIotivityHandler::deleteObject>(cb, user_data);
	DeleteCallback deleteCallback = bind(&icIotivityHandler::deleteObject::onDelete,
			object, placeholders::_1, placeholders::_2);

	ret = ocResource->deleteResource(deleteCallback);
	if (OC_STACK_OK != ret) {
		ERR("deleteResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_observe(iotcon_client_h resource,
		iotcon_observe_type_e observe_type,
		iotcon_query_h query,
		iotcon_on_observe_cb cb,
		void *user_data)
{
	OCStackResult ret;
	OCResource::Ptr ocResource;
	ObserveType observeType;
	QueryParamsMap queryParams;

	if (query)
		iotcon_query_foreach(query, _ic_ioty_accumulate_query_map, (void *)&queryParams);

	if (IOTCON_OBSERVE == observe_type) {
		observeType = ObserveType::Observe;
	} else if (IOTCON_OBSERVE_ALL == observe_type) {
		observeType = ObserveType::ObserveAll;
	} else {
		ERR("Invalid observe_type");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ocResource = _ic_ioty_create_oc_resource(resource);

	resource_handle *obs_h = new resource_handle();
	obs_h->ocResource = ocResource;
	resource->observe_handle = (void *)obs_h;

	shared_ptr<icIotivityHandler::observeObject> object
		= make_shared<icIotivityHandler::observeObject>(cb, user_data);
	ObserveCallback observeCallback = bind(&icIotivityHandler::observeObject::onObserve,
			object, placeholders::_1, placeholders::_2, placeholders::_3,
			placeholders::_4);
	ret = ocResource->observe(observeType, queryParams, observeCallback);
	if (OC_STACK_OK != ret) {
		ERR("observe() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}

extern "C" int ic_ioty_cancel_observe(iotcon_client_h resource)
{
	OCStackResult ret;
	resource_handle *resource_h = (resource_handle *)resource->observe_handle;

	OCResource::Ptr ocResource = resource_h->ocResource;
	delete (resource_handle *)resource->observe_handle;
	resource->observe_handle = NULL;

	ret = ocResource->cancelObserve(QualityOfService::HighQos);
	if (OC_STACK_OK != ret) {
		ERR("cancelObserve() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}
