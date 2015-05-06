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
#include <glib.h>
#include <OCApi.h>
#include <OCPlatform.h>

extern "C" {
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-rep.h"
#include "ic-handler.h"
#include "ic-struct.h"
}
#include "ic-ioty-rep.h"
#include "ic-ioty.h"

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
		iotcon_presence_handle_cb presence_handler;
		void *cb_data;

	public:
		presenceObject(iotcon_presence_handle_cb user_cb, void *user_data)
		{
			presence_handler = user_cb;
			cb_data = user_data;
		}

		void presenceHandler(OCStackResult result, const unsigned int nonce,
				const string& hostAddress)
		{
			iotcon_error_e ret;

			if (OC_STACK_OK != result) {
				ERR("subscribePresence() result Fail(%d)", result);
				ret = IOTCON_ERR_IOTIVITY;
			}
			else {
				ret = IOTCON_ERR_NONE;
			}

			if (presence_handler)
				presence_handler(ret, nonce, hostAddress.c_str(), cb_data);
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
			iotcon_resource_s resource_s = {0};

			resource_s.resource_uri = ic_utils_strdup(resource->uri().c_str());
			resource_s.resource_host = ic_utils_strdup(resource->host().c_str());
			resource_s.is_observable = resource->isObservable();
			resource_s.resource_types = iotcon_resource_types_new();
			resource_s.resource_interfaces = iotcon_resource_interfaces_new();

			vector<string> resource_types = resource->getResourceTypes();
			for (string &resource_type : resource_types)
				resource_s.resource_types
					= iotcon_resource_types_insert(resource_s.resource_types,
							ic_utils_strdup(resource_type.c_str()));

			vector<string> resource_interfaces = resource->getResourceInterfaces();
			for (string &resource_interface : resource_interfaces)
				resource_s.resource_interfaces
					= g_list_append(resource_s.resource_interfaces,
							ic_utils_strdup(resource_interface.c_str()));

			if (found_resource)
				found_resource(&resource_s, cb_data);

			free(resource_s.resource_uri);
			free(resource_s.resource_host);
			iotcon_resource_types_free(resource_s.resource_types);
			iotcon_resource_interfaces_free(resource_s.resource_interfaces);
		}
	};

	class getObject
	{
	private:
		iotcon_on_get_cb on_get;
		void *cb_data;

	public:
		getObject(iotcon_on_get_cb user_cb, void *user_data)
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

			if (OC_STACK_OK == eCode)
				res = IOTCON_ERR_NONE;
			else
				res = IOTCON_ERR_IOTIVITY;

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_get)
				on_get(options, repr, res, cb_data);

			iotcon_options_free(options);
		}
	};

	class putObject
	{
	private:
		iotcon_on_put_cb on_put;
		void *cb_data;

	public:
		putObject(iotcon_on_put_cb user_cb, void *user_data)
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

			if (OC_STACK_OK == eCode)
				res = IOTCON_ERR_NONE;
			else
				res = IOTCON_ERR_IOTIVITY;

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_put)
				on_put(options, repr, res, cb_data);

			iotcon_options_free(options);
		}
	};

	class postObject
	{
	private:
		iotcon_on_post_cb on_post;
		void *cb_data;

	public:
		postObject(iotcon_on_post_cb user_cb, void *user_data)
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

			if (OC_STACK_OK == eCode)
				res = IOTCON_ERR_NONE;
			else
				res = IOTCON_ERR_IOTIVITY;

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_post)
				on_post(options, repr, res, cb_data);

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

			if (OC_STACK_OK == eCode)
				res = IOTCON_ERR_NONE;
			else
				res = IOTCON_ERR_IOTIVITY;

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

			if (OC_STACK_OK == eCode)
				res = IOTCON_ERR_NONE;
			else
				res = IOTCON_ERR_IOTIVITY;

			options = iotcon_options_new();
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				iotcon_options_insert(options, option.getOptionID(),
						option.getOptionData().c_str());
			}

			repr = ic_ioty_repr_generate_repr(ocRep);

			if (on_observe)
				on_observe(options, repr, res, sequenceNumber, cb_data);

			iotcon_options_free(options);
		}
	};
}

extern "C" void ic_iotivity_config(const char *addr, unsigned short port)
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

static OCEntityHandlerResult _entity_handler(shared_ptr<OCResourceRequest> request)
{
	FN_CALL;
	HeaderOptions headerOptions;
	QueryParamsMap queryParams;
	resource_handler_s *temp_res = NULL;
	iotcon_request_s request_s = {0};

	temp_res = ic_get_resource_handler_data(request->getResourceHandle());
	if (NULL == temp_res) {
		ERR("No Resource Handler");
		return OC_EH_ERROR;
	}

	request_s.query = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	if (NULL == request_s.query) {
		ERR("g_hash_table_new_full() Fail");
		return OC_EH_ERROR;
	}

	map<string,string>::iterator it;
	queryParams = request->getQueryParameters();
	for (it = queryParams.begin(); it != queryParams.end(); ++it) {
		DBG("key=%s value=%s", it->first.c_str(), it->second.c_str());
		g_hash_table_insert(request_s.query, ic_utils_strdup(it->first.c_str()),
				ic_utils_strdup(it->second.c_str()));
	}

	request_s.header_options = iotcon_options_new();
	if (NULL == request_s.header_options) {
		ERR("iotcon_options_new() Fail");
		return OC_EH_ERROR;
	}

	headerOptions = request->getHeaderOptions();
	if (0 < headerOptions.size()) {
		for (auto it1 = headerOptions.begin(); it1 != headerOptions.end(); ++it1) {
			DBG("OptionID=%d, OptionData=%s",
					it1->getOptionID(), it1->getOptionData().c_str());
			iotcon_options_insert(request_s.header_options, it1->getOptionID(),
					it1->getOptionData().c_str());
		}
	}

	OCRepresentation ocRep = request->getResourceRepresentation();
	if (0 < ocRep.numberOfAttributes()) {
		DBG("numberOfAttributes : %d", ocRep.numberOfAttributes());
		request_s.repr = ic_ioty_repr_generate_repr(ocRep);
		if (NULL == request_s.repr) {
			ERR("request_s.repr is NULL");
			return OC_EH_ERROR;
		}
	}

	request_s.request_type = ic_utils_strdup(request->getRequestType().c_str());
	if (NULL == request_s.request_type) {
		ERR("ic_utils_strdup() Fail");
		iotcon_options_free(request_s.header_options);
		iotcon_query_free(request_s.query);
		return OC_EH_ERROR;
	}

	request_s.res_uri = ic_utils_strdup(request->getResourceUri().c_str());
	if (NULL == request_s.request_type) {
		ERR("ic_utils_strdup() Fail");
		free(request_s.request_type);
		iotcon_options_free(request_s.header_options);
		iotcon_query_free(request_s.query);
		return OC_EH_ERROR;
	}

	request_s.request_handler_flag = request->getRequestHandlerFlag();
	request_s.request_handle = (iotcon_request_h)request->getRequestHandle();
	request_s.resource_handle = (iotcon_resource_h)request->getResourceHandle();
	ObservationInfo observationInfo = request->getObservationInfo();

	request_s.observation_info.action = (iotcon_osbserve_action_e)observationInfo.action;
	request_s.observation_info.obs_id = (iotcon_observation_id)observationInfo.obsId;
	DBG("obs_info.obsId=%d", observationInfo.obsId);

	/* call handler_cb */
	if (temp_res->rest_api_cb) {
		temp_res->rest_api_cb(&request_s);
	}
	else {
		WARN("temp_res->rest_api_cb is null");
	}

	free(request_s.request_type);
	free(request_s.res_uri);
	if (request_s.repr) /* To avoid unnecessary ERR log (repr could be NULL) */
		iotcon_repr_free(request_s.repr);
	iotcon_options_free(request_s.header_options);
	iotcon_query_free(request_s.query);

	return OC_EH_OK;
}


extern "C" void* ic_ioty_register_res(const char *uri, const char *rt,
		iotcon_interface_e iface, iotcon_resource_property_e rt_property)
{
	OCStackResult ret;
	string resUri;
	string resType;
	string resInterface;
	OCResourceHandle handle;

	resUri = uri;
	resType = rt;

	if (IOTCON_INTERFACE_LINK == iface)
		resInterface = LINK_INTERFACE;
	else if (IOTCON_INTERFACE_BATCH == iface)
		resInterface = BATCH_INTERFACE;
	else if (IOTCON_INTERFACE_GROUP == iface)
		resInterface = GROUP_INTERFACE;
	else
		resInterface = DEFAULT_INTERFACE;


	ret = registerResource(handle, resUri, resType, resInterface, _entity_handler,
			(unsigned int)rt_property);
	if (OC_STACK_OK != ret) {
		ERR("registerResource Fail(%d)", ret);
		return NULL;
	}

	return handle;
}


extern "C" int ic_ioty_unregister_res(const iotcon_resource_h resource_handle)
{
	OCResourceHandle resourceHandle = resource_handle;

	try {
		OCStackResult result = unregisterResource(resourceHandle);
		if(OC_STACK_OK != result) {
			ERR("OCPlatform::unregisterResource Fail(%d)", result);
			return IOTCON_ERR_IOTIVITY;
		}
	}
	catch(OCException& e) {
		ERR("unregisterResource() Fail(%s)", e.what());
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}


extern "C" int ic_ioty_bind_iface_to_res(const iotcon_resource_h resource_handle,
		const char *interface_type)
{
	OCStackResult ret = OC_STACK_ERROR;
	OCResourceHandle resourceHandle = resource_handle;

	ret = bindInterfaceToResource(resourceHandle, interface_type);
	if (OC_STACK_OK != ret) {
		ERR("bindInterfaceToResource() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}


extern "C" int ic_ioty_bind_type_to_res(const iotcon_resource_h resource_handle,
		const char *resource_type)
{
	OCStackResult ret;
	OCResourceHandle resourceHandle = resource_handle;

	ret = bindTypeToResource(resourceHandle, resource_type);
	if (OC_STACK_OK != ret) {
		ERR("bindTypeToResource() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_bind_res(iotcon_resource_h parent, iotcon_resource_h child)
{
	OCStackResult ret;
	OCResourceHandle p_handle = parent;
	OCResourceHandle c_handle = child;

	ret = OCBindResource(p_handle, c_handle);
	if (OC_STACK_OK != ret) {
		ERR("OCBindResource() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_register_device_info(iotcon_device_info_s *device_info)
{
	FN_CALL;
	OCStackResult ret;

	OCDeviceInfo deviceInfo = {0};
	deviceInfo.deviceName = ic_utils_strdup(device_info->device_name);
	deviceInfo.hostName = ic_utils_strdup(device_info->host_name);
	deviceInfo.deviceUUID = ic_utils_strdup(device_info->device_uuid);
	deviceInfo.contentType = ic_utils_strdup(device_info->content_type);
	deviceInfo.version = ic_utils_strdup(device_info->version);
	deviceInfo.manufacturerName = ic_utils_strdup(device_info->manufacturer_name);
	deviceInfo.manufacturerUrl = ic_utils_strdup(device_info->manufacturer_url);
	deviceInfo.modelNumber = ic_utils_strdup(device_info->model_number);
	deviceInfo.dateOfManufacture = ic_utils_strdup(device_info->date_of_manufacture);
	deviceInfo.platformVersion = ic_utils_strdup(device_info->platform_version);
	deviceInfo.firmwareVersion = ic_utils_strdup(device_info->firmware_version);
	deviceInfo.supportUrl = ic_utils_strdup(device_info->support_url);

	ret = registerDeviceInfo(deviceInfo);
	if (OC_STACK_OK != ret) {
		ERR("registerDeviceInfo() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}


extern "C" int ic_ioty_get_device_info(char *host, char *uri)
{
	OCStackResult ret;
	string resHost;
	string resUri;

	resHost = host;
	resUri = uri;

	/* register device_cb
	 to monitor upper apps who wnat to know other device infomation */
	ret = getDeviceInfo(resHost, resUri, ic_ioty_repr_found_device_cb);
	if (OC_STACK_OK != ret) {
		ERR("getDeviceInfo() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}


extern "C" int ic_ioty_send_notify(struct ic_res_response_s *resp,
		iotcon_observers observers)
{
	int ret;
	ObservationIds obsIds;
	OCResourceHandle resHandle;
	string interface;

	RETV_IF(NULL == resp, IOTCON_ERR_PARAM);
	RETV_IF(NULL == resp->repr, IOTCON_ERR_PARAM);
	RETV_IF(NULL == observers, IOTCON_ERR_PARAM);

	resHandle = resp->resource_handle;

	RETV_IF(NULL == resp->repr, IOTCON_ERR_PARAM);

	OCRepresentation ocRep = ic_ioty_repr_parse(resp->repr);

	GList *node = g_list_first(observers);
	while (node) {
		iotcon_observation_info_s *temp = (iotcon_observation_info_s*)node->data;
		obsIds.push_back(temp->obs_id);

		node = node->next;
	}

	shared_ptr<OCResourceResponse> resourceResponse(new OCResourceResponse());
	resourceResponse->setErrorCode(resp->error_code);

	switch (resp->interface) {
	case IOTCON_INTERFACE_GROUP:
		interface = GROUP_INTERFACE;
		break;
	case IOTCON_INTERFACE_BATCH:
		interface = BATCH_INTERFACE;
		break;
	case IOTCON_INTERFACE_LINK:
		interface = LINK_INTERFACE;
		break;
	case IOTCON_INTERFACE_DEFAULT:
	default:
		interface = DEFAULT_INTERFACE;
	}

	resourceResponse->setResourceRepresentation(ocRep, interface);

	ret = notifyListOfObservers(resHandle, obsIds, resourceResponse);
	if (OC_STACK_NO_OBSERVERS == ret) {
		ERR("No More observers, stopping notifications");
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}


extern "C" int ic_ioty_send_res_response_data(struct ic_res_response_s *resp)
{
	FN_CALL;
	string interface;
	int ret = OC_STACK_ERROR;

	RETV_IF(NULL == resp, IOTCON_ERR_PARAM);

	RETV_IF(NULL == resp->repr, IOTCON_ERR_PARAM);

	OCRepresentation ocRep = ic_ioty_repr_parse(resp->repr);
	iotcon_repr_free(resp->repr);

	auto pResponse = make_shared<OCResourceResponse>();
	if (pResponse) {
		pResponse->setRequestHandle(resp->request_handle);
		pResponse->setResourceHandle(resp->resource_handle);
		pResponse->setErrorCode(resp->error_code);
		pResponse->setResponseResult((OCEntityHandlerResult)resp->result);

		switch (resp->interface) {
		case IOTCON_INTERFACE_GROUP:
			interface = GROUP_INTERFACE;
			break;
		case IOTCON_INTERFACE_BATCH:
			interface = BATCH_INTERFACE;
			break;
		case IOTCON_INTERFACE_LINK:
			interface = LINK_INTERFACE;
			break;
		case IOTCON_INTERFACE_DEFAULT:
		default:
			interface = DEFAULT_INTERFACE;
		}

		pResponse->setResourceRepresentation(ocRep, interface);

		ret = sendResponse(pResponse);
		if (OC_STACK_OK != ret) {
			ERR("sendResponse() Fail(%d)", ret);
			return IOTCON_ERR_IOTIVITY;
		}
	}

	FN_END;

	return IOTCON_ERR_NONE;

}

extern "C" iotcon_presence_h ic_ioty_subscribe_presence(const char *host_address,
		iotcon_presence_handle_cb presence_handler_cb, void *user_data)
{
	OCStackResult ret;
	iotcon_presence_h presence_handle = NULL;

	shared_ptr<icIotivityHandler::presenceObject> object
		= make_shared<icIotivityHandler::presenceObject>(presence_handler_cb, user_data);
	SubscribeCallback subscribeCallback
		= bind(&icIotivityHandler::presenceObject::presenceHandler, object,
				placeholders::_1, placeholders::_2, placeholders::_3);

	ret = subscribePresence(presence_handle, host_address, subscribeCallback);
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
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_start_presence(const unsigned int time_to_live)
{
	try {
		startPresence(time_to_live);
	} catch (OCException e) {
		ERR("startPresence() Fail(%s)", e.what());
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_stop_presence()
{
	try {
		stopPresence();
	} catch (OCException e) {
		ERR("stopPresence() Fail(%s)", e.what());
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_find_resource(const char *host, const char *resource_name,
		iotcon_found_resource_cb found_resource_cb, void *user_data)
{
	OCStackResult ret;

	shared_ptr<icIotivityHandler::findObject> object
		= make_shared<icIotivityHandler::findObject>(found_resource_cb, user_data);
	FindCallback findCallback = bind(&icIotivityHandler::findObject::foundResource,
			object, placeholders::_1);

	ret = findResource(host, resource_name, findCallback);
	if (OC_STACK_OK != ret) {
		ERR("findResource() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

static void _change_options(unsigned short id, char *data, void *user_data)
{
	HeaderOptions *options = static_cast<HeaderOptions *>(user_data);
	HeaderOption::OCHeaderOption option(id, data);
	(*options).push_back(option);
}

static OCResource::Ptr _create_oc_resource(iotcon_resource_s resource)
{
	string host = resource.resource_host;
	string uri = resource.resource_uri;
	vector<string> resource_types;
	vector<string> resource_ifs;

	HeaderOptions header_options;

	RETV_IF(NULL == resource.resource_host, NULL);
	RETV_IF(NULL == resource.resource_uri, NULL);

	GList *node = resource.resource_types;
	for (node = g_list_first(node); node; node = g_list_next(node)) {
		string resource_type_str = (char *)node->data;
		resource_types.push_back(resource_type_str);
	}

	node = resource.resource_interfaces;
	for (node = g_list_first(node); node; node = g_list_next(node)) {
		string resource_if_str = (char *)node->data;
		resource_ifs.push_back(resource_if_str);
	}

	OCResource::Ptr ocResource = constructResourceObject(host, uri,
			resource.is_observable, resource_types, resource_ifs);

	if (resource.header_options) {
		iotcon_options_foreach(resource.header_options, _change_options,
				(void *)&header_options);
		ocResource->setHeaderOptions(header_options);
	}

	return ocResource;
}

extern "C" int ic_ioty_get(iotcon_resource_s resource,
		iotcon_query query, iotcon_on_get_cb on_get_cb, void *user_data)
{
	FN_CALL;
	OCStackResult ret;
	OCResource::Ptr ocResource = NULL;

	QueryParamsMap queryParams;

	GHashTableIter iter;
	gpointer key, value;

	if (query) {
		g_hash_table_iter_init(&iter, query);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			string keyStr = (char *)key;
			string valueStr = (char *)value;
			queryParams[keyStr] = valueStr;
		}
	}

	ocResource = _create_oc_resource(resource);

	shared_ptr<icIotivityHandler::getObject> object
		= make_shared<icIotivityHandler::getObject>(on_get_cb, user_data);
	GetCallback getCallback = bind(&icIotivityHandler::getObject::onGet, object,
			placeholders::_1, placeholders::_2, placeholders::_3);
	ret = ocResource->get(queryParams, getCallback);
	if (OC_STACK_OK != ret) {
		ERR("get() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_put(iotcon_resource_s resource,
		iotcon_repr_h repr,
		iotcon_query query,
		iotcon_on_put_cb on_put_cb,
		void *user_data)
{
	FN_CALL;
	OCStackResult ret;

	OCResource::Ptr ocResource = NULL;
	OCRepresentation ocRep;
	QueryParamsMap queryParams;

	GHashTableIter iter;
	gpointer key, value;

	if (query) {
		g_hash_table_iter_init(&iter, query);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			string keyStr = (char *)key;
			string valueStr = (char *)value;
			queryParams[keyStr] = valueStr;
		}
	}

	ocRep = ic_ioty_repr_parse(repr);

	ocResource = _create_oc_resource(resource);

	shared_ptr<icIotivityHandler::putObject> object
		= make_shared<icIotivityHandler::putObject>(on_put_cb, user_data);
	PutCallback putCallback = bind(&icIotivityHandler::putObject::onPut, object,
			placeholders::_1, placeholders::_2, placeholders::_3);

	ret = ocResource->put(ocRep, queryParams, putCallback);
	if (OC_STACK_OK != ret) {
		ERR("put() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_post(iotcon_resource_s resource,
		iotcon_repr_h repr,
		iotcon_query query,
		iotcon_on_post_cb on_post_cb,
		void *user_data)
{
	FN_CALL;
	OCStackResult ret;
	GHashTableIter iter;
	gpointer key, value;
	QueryParamsMap queryParams;
	OCRepresentation ocRep;
	OCResource::Ptr ocResource = NULL;

	if (query) {
		g_hash_table_iter_init(&iter, query);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			string keyStr = (char *)key;
			string valueStr = (char *)value;
			queryParams[keyStr] = valueStr;
		}
	}

	ocRep = ic_ioty_repr_parse(repr);

	ocResource = _create_oc_resource(resource);

	shared_ptr<icIotivityHandler::postObject> object
		= make_shared<icIotivityHandler::postObject>(on_post_cb, user_data);
	PostCallback postCallback = bind(&icIotivityHandler::postObject::onPost, object,
			placeholders::_1, placeholders::_2, placeholders::_3);

	ret = ocResource->post(ocRep, queryParams, postCallback);
	if (OC_STACK_OK != ret) {
		ERR("post() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_delete_res(iotcon_resource_s resource,
		iotcon_on_delete_cb on_delete_cb, void *user_data)
{
	FN_CALL;
	OCStackResult ret;
	OCResource::Ptr ocResource = NULL;

	ocResource = _create_oc_resource(resource);

	shared_ptr<icIotivityHandler::deleteObject> object
		= make_shared<icIotivityHandler::deleteObject>(on_delete_cb, user_data);
	DeleteCallback deleteCallback = bind(&icIotivityHandler::deleteObject::onDelete,
			object, placeholders::_1, placeholders::_2);

	ret = ocResource->deleteResource(deleteCallback);
	if (OC_STACK_OK != ret) {
		ERR("deleteResource() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_observe(iotcon_resource_s *resource,
		iotcon_observe_type_e observe_type,
		iotcon_query query,
		iotcon_on_observe_cb on_observe_cb,
		void *user_data)
{
	OCStackResult ret;

	OCResource::Ptr ocResource = NULL;
	ObserveType observeType;

	QueryParamsMap queryParams;
	GHashTableIter iter;
	gpointer key, value;

	if (query) {
		g_hash_table_iter_init(&iter, query);
		while (g_hash_table_iter_next(&iter, &key, &value)) {
			string keyStr = (char *)key;
			string valueStr = (char *)value;
			queryParams[keyStr] = valueStr;
		}
	}

	if (IOTCON_OBSERVE == observe_type) {
		observeType = ObserveType::Observe;
	}
	else if (IOTCON_OBSERVE_ALL == observe_type) {
		observeType = ObserveType::ObserveAll;
	}
	else {
		ERR("Invalid observe_type");
		return IOTCON_ERR_PARAM;
	}

	ocResource = _create_oc_resource(*resource);

	resource_handle *obs_h = new resource_handle();
	obs_h->ocResource = ocResource;
	resource->observe_handle = (void *)obs_h;

	shared_ptr<icIotivityHandler::observeObject> object
		= make_shared<icIotivityHandler::observeObject>(on_observe_cb, user_data);
	ObserveCallback observeCallback = bind(&icIotivityHandler::observeObject::onObserve,
			object, placeholders::_1, placeholders::_2, placeholders::_3,
			placeholders::_4);
	ret = ocResource->observe(observeType, queryParams, observeCallback);
	if (OC_STACK_OK != ret) {
		ERR("observe() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}

extern "C" int ic_ioty_cancel_observe(iotcon_resource_s resource)
{
	OCStackResult ret;

	OCResource::Ptr ocResource = ((resource_handle *)resource.observe_handle)->ocResource;
	delete (resource_handle *)resource.observe_handle;
	resource.observe_handle = NULL;

	ret = ocResource->cancelObserve();
	if (OC_STACK_OK != ret) {
		ERR("cancelObserve() Fail(%d)", ret);
		return IOTCON_ERR_IOTIVITY;
	}

	return IOTCON_ERR_NONE;
}
