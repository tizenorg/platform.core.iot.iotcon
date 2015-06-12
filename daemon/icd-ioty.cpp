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
#include <stdio.h>
#include <glib.h>
#include <OCApi.h>
#include <OCPlatform.h>

extern "C" {
#include "iotcon.h"
#include "ic-utils.h"
#include "icd.h"
#include "icd-ioty.h"
#include "icd-dbus.h"
#include "icd-ioty-repr.h"
}

#define ICD_COAP "coap://"
#define ICD_UNICAST_RESOURCE_DISCOVERY "/oc/core"
#define ICD_MULTICAST_RESOURCE_DISCOVERY "/oc/core"
#define ICD_DEVICE_DISCOVERY "/oc/core/d"

using namespace std;
using namespace OC;
using namespace OCPlatform;

struct resource_handle {
	OCResource::Ptr ocResource;
};

namespace icdIotivityHandler {
	class getObject
	{
	private:
		string sig_name;

	public:
		getObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void onGet(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode)
		{
			FN_CALL;
			int res, ret;
			GVariant *value;
			GVariantBuilder *options;
			GVariantBuilder *repr;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else {
				ERR("get() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = g_variant_builder_new(G_VARIANT_TYPE("a(qs)"));
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				g_variant_builder_add(options, "(qs)", option.getOptionID(),
						option.getOptionData().c_str());
			}

			ret = icd_ioty_repr_generate_gvariant_builder(ocRep, &repr);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icd_ioty_repr_generate_gvariant_builder() Fail(%d)", ret);
				g_variant_builder_unref(options);
				return;
			}

			value = g_variant_new("(a(qs)asi)", options, repr, res);
			g_variant_builder_unref(options);
			g_variant_builder_unref(repr);

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};

	class putObject
	{
	private:
		string sig_name;

	public:
		putObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void onPut(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode)
		{
			FN_CALL;
			int res, ret;
			GVariant *value;
			GVariantBuilder *options;
			GVariantBuilder *repr;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else {
				ERR("put() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = g_variant_builder_new(G_VARIANT_TYPE("a(qs)"));
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				g_variant_builder_add(options, "(qs)", option.getOptionID(),
						option.getOptionData().c_str());
			}

			ret = icd_ioty_repr_generate_gvariant_builder(ocRep, &repr);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icd_ioty_repr_generate_gvariant_builder() Fail(%d)", ret);
				g_variant_builder_unref(options);
				return;
			}

			value = g_variant_new("(a(qs)asi)", options, repr, res);
			g_variant_builder_unref(options);
			g_variant_builder_unref(repr);

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};

	class postObject
	{
	private:
		string sig_name;

	public:
		postObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void onPost(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode)
		{
			FN_CALL;
			int res, ret;
			GVariant *value;
			GVariantBuilder *options;
			GVariantBuilder *repr;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else if (OC_STACK_RESOURCE_CREATED == eCode) {
				res = IOTCON_RESPONSE_RESULT_RESOURCE_CREATED;
			} else {
				ERR("post() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = g_variant_builder_new(G_VARIANT_TYPE("a(qs)"));
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				g_variant_builder_add(options, "(qs)", option.getOptionID(),
						option.getOptionData().c_str());
			}

			ret = icd_ioty_repr_generate_gvariant_builder(ocRep, &repr);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icd_ioty_repr_generate_gvariant_builder() Fail(%d)", ret);
				g_variant_builder_unref(options);
				return;
			}

			value = g_variant_new("(a(qs)asi)", options, repr, res);
			g_variant_builder_unref(options);
			g_variant_builder_unref(repr);

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};

	class deleteObject
	{
	private:
		string sig_name;

	public:
		deleteObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void onDelete(const HeaderOptions& headerOptions, const int eCode)
		{
			int res;
			GVariant *value;
			GVariantBuilder *options;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else if (OC_STACK_RESOURCE_DELETED == eCode) {
				res = IOTCON_RESPONSE_RESULT_RESOURCE_DELETED;
			} else {
				ERR("deleteResource() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}

			options = g_variant_builder_new(G_VARIANT_TYPE("a(qs)"));
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				g_variant_builder_add(options, "(qs)", option.getOptionID(),
						option.getOptionData().c_str());
			}

			value = g_variant_new("(a(qs)i)", options, res);
			g_variant_builder_unref(options);

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};

	class observeObject
	{
	private:
		string sig_name;

	public:
		observeObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void onObserve(const HeaderOptions& headerOptions, const OCRepresentation& ocRep,
				const int eCode, const int sequenceNumber)
		{
			FN_CALL;
			int res, ret;
			GVariant *value;
			GVariantBuilder *options;
			GVariantBuilder *repr;

			if (OC_STACK_OK == eCode) {
				res = IOTCON_RESPONSE_RESULT_OK;
			} else {
				ERR("observe() Fail(%d)", eCode);
				res = IOTCON_RESPONSE_RESULT_ERROR;
			}
			options = g_variant_builder_new(G_VARIANT_TYPE("a(qs)"));
			for (HeaderOption::OCHeaderOption option : headerOptions) {
				g_variant_builder_add(options, "(qs)", option.getOptionID(),
						option.getOptionData().c_str());
			}

			ret = icd_ioty_repr_generate_gvariant_builder(ocRep, &repr);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icd_ioty_repr_generate_gvariant_builder() Fail(%d)", ret);
				g_variant_builder_unref(options);
				return;
			}

			value = g_variant_new("(a(qs)asii)", options, repr, res, sequenceNumber);
			g_variant_builder_unref(options);
			g_variant_builder_unref(repr);

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};

	class findObject
	{
	private:
		string sig_name;

	public:
		findObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void foundResource(shared_ptr<OCResource> resource)
		{
			FN_CALL;
			int ifaces = 0;
			GVariant *value;
			GVariantBuilder *builder;
			string resource_host;

			builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
			vector<string> resource_types = resource->getResourceTypes();
			if (0 < resource_types.size()) {
				for (string &resource_type : resource_types) {
					INFO("type : %s", resource_type.c_str());
					g_variant_builder_add(builder, "s", resource_type.c_str());
				}
			}

			vector<string> resource_interfaces = resource->getResourceInterfaces();
			for (string &resource_interface : resource_interfaces) {
				if (IC_STR_EQUAL == resource_interface.compare(DEFAULT_INTERFACE))
					ifaces |= IOTCON_INTERFACE_DEFAULT;

				if (IC_STR_EQUAL == resource_interface.compare(BATCH_INTERFACE))
					ifaces |= IOTCON_INTERFACE_BATCH;

				if (IC_STR_EQUAL == resource_interface.compare(LINK_INTERFACE))
					ifaces |= IOTCON_INTERFACE_LINK;

				if (IC_STR_EQUAL == resource_interface.compare(GROUP_INTERFACE))
					ifaces |= IOTCON_INTERFACE_GROUP;
			}

			resource_host = resource->host();
			resource_host.erase(0, strlen(ICD_COAP));

			value = g_variant_new("(ssiasi)",
					resource->uri().c_str(),
					resource_host.c_str(),
					resource->isObservable(),
					builder,
					ifaces);
			g_variant_builder_unref(builder);

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};

	class deviceObject
	{
	private:
		string sig_name;

	public:
		deviceObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void receivedDeviceInfo(const OCRepresentation& ocRep)
		{
			FN_CALL;
			GVariant *value;

			string contentType;
			string dateOfManufacture;
			string deviceName;
			string deviceUUID;
			string firmwareVersion;
			string hostName;
			string manufacturerName;
			string manufacturerUrl;
			string modelNumber;
			string platformVersion;
			string supportUrl;
			string version;

			ocRep.getValue("dn", deviceName);
			ocRep.getValue("hn", hostName);
			ocRep.getValue("di", deviceUUID);
			ocRep.getValue("ct", contentType);
			ocRep.getValue("icv", version);
			ocRep.getValue("mnmn", manufacturerName);
			ocRep.getValue("mnml", manufacturerUrl);
			ocRep.getValue("mnmo", modelNumber);
			ocRep.getValue("mndt", dateOfManufacture);
			ocRep.getValue("mnpv", platformVersion);
			ocRep.getValue("mnfv", firmwareVersion);
			ocRep.getValue("mnsl", supportUrl);

			value = g_variant_new("(ssssssssssss)",
					deviceName.c_str(),
					hostName.c_str(),
					deviceUUID.c_str(),
					contentType.c_str(),
					version.c_str(),
					manufacturerName.c_str(),
					manufacturerUrl.c_str(),
					modelNumber.c_str(),
					dateOfManufacture.c_str(),
					platformVersion.c_str(),
					firmwareVersion.c_str(),
					supportUrl.c_str());

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};

	class presenceObject
	{
	private:
		string sig_name;

	public:
		presenceObject(const char *user_data)
		{
			sig_name = user_data;
		}

		void presenceHandler(OCStackResult result, const unsigned int nonce,
				const string& hostAddress)
		{
			int res;
			GVariant *value;

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

			value = g_variant_new("(ius)", res, nonce, hostAddress.c_str());

			icd_dbus_emit_signal(sig_name.c_str(), value);
		}
	};
}


extern "C" void icd_ioty_config(const char *addr, unsigned short port)
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


static OCEntityHandlerResult _icd_ioty_request_handler(
		shared_ptr<OCResourceRequest> request)
{
	FN_CALL;
	int ret;
	int types = 0;
	int observer_id;
	int observe_action;
	int request_handle;
	int resource_handle;
	char sig_name[100] = {0};
	const char *uri = NULL;
	const char *request_type = NULL;
	GVariant *value;
	GVariantBuilder *options;
	GVariantBuilder *query;
	GVariantBuilder *repr;
	HeaderOptions headerOptions;
	QueryParamsMap queryParams;
	OCRepresentation ocRep;

	/* request type */
	if (RequestFlag & request->getRequestHandlerFlag()) {
		request_type = request->getRequestType().c_str();
		if (NULL == request_type) {
			ERR("request_type is NULL");
			return OC_EH_ERROR;
		}

		if (IC_STR_EQUAL == strcmp("GET", request_type))
			types = IOTCON_REQUEST_GET;
		else if (IC_STR_EQUAL == strcmp("PUT", request_type))
			types = IOTCON_REQUEST_PUT;
		else if (IC_STR_EQUAL == strcmp("POST", request_type))
			types = IOTCON_REQUEST_POST;
		else if (IC_STR_EQUAL == strcmp("DELETE", request_type))
			types = IOTCON_REQUEST_DELETE;
	}

	if (ObserverFlag & request->getRequestHandlerFlag())
		types |= IOTCON_REQUEST_OBSERVE;

	/* uri */
	uri = request->getResourceUri().c_str();
	if (NULL == uri) {
		ERR("uri is NULL");
		return OC_EH_ERROR;
	}

	/* header options */
	headerOptions = request->getHeaderOptions();
	options = g_variant_builder_new(G_VARIANT_TYPE("a(qs)"));
	if (0 < headerOptions.size()) {
		for (auto it : headerOptions) {
			DBG("OptionID = %d, OptionData = %s",
					it.getOptionID(), it.getOptionData().c_str());
			g_variant_builder_add(options, "(qs)", it.getOptionID(),
					it.getOptionData().c_str());
		}
	}

	/* query */
	queryParams = request->getQueryParameters();
	query = g_variant_builder_new(G_VARIANT_TYPE("a(ss)"));
	if (0 < queryParams.size()) {
		for (auto it : queryParams) {
			DBG("key = %s value = %s", it.first.c_str(), it.second.c_str());
			g_variant_builder_add(query, "(ss)", it.first.c_str(), it.second.c_str());
		}
	}

	/* observation info */
	ObservationInfo observationInfo = request->getObservationInfo();
	observe_action = (int)observationInfo.action;
	observer_id = observationInfo.obsId;
	DBG("obs_info.obsId=%d", observationInfo.obsId);

	/* Representation */
	ocRep = request->getResourceRepresentation();
	ret = icd_ioty_repr_generate_gvariant_builder(ocRep, &repr);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_repr_generate_gvariant_builder() Fail(%d)", ret);
		g_variant_builder_unref(options);
		g_variant_builder_unref(query);
		return OC_EH_ERROR;
	}

	/* handle */
	request_handle = GPOINTER_TO_INT(request->getRequestHandle());
	resource_handle = GPOINTER_TO_INT(request->getResourceHandle());

	value = g_variant_new("(isa(qs)a(ss)iiasii)", types, uri, options, query,
			observe_action, observer_id, repr, request_handle, resource_handle);

	g_variant_builder_unref(options);
	g_variant_builder_unref(query);
	g_variant_builder_unref(repr);

	snprintf(sig_name, sizeof(sig_name), "reg%u", resource_handle);
	icd_dbus_emit_signal(sig_name, value);

	return OC_EH_OK;
}


extern "C" OCResourceHandle icd_ioty_register_resource(const char *uri,
		const char* const* res_types, int ifaces, uint8_t properties)
{
	OCStackResult ret;
	string resUri;
	string resType;
	string resInterface;
	OCResourceHandle handle;

	resUri = uri;
	resType = string(res_types[0]);

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
			_icd_ioty_request_handler, properties);
	if (OC_STACK_OK != ret) {
		ERR("registerResource Fail(%d)", ret);
		return NULL;
	}

	for (int i = 1; res_types[i]; i++)
		icd_ioty_bind_type(handle, res_types[i]);

	if (IOTCON_INTERFACE_DEFAULT & ifaces)
		icd_ioty_bind_interface(handle, IOTCON_INTERFACE_DEFAULT);

	if (IOTCON_INTERFACE_LINK & ifaces)
		icd_ioty_bind_interface(handle, IOTCON_INTERFACE_LINK);

	if (IOTCON_INTERFACE_BATCH & ifaces)
		icd_ioty_bind_interface(handle, IOTCON_INTERFACE_BATCH);

	if (IOTCON_INTERFACE_GROUP & ifaces)
		icd_ioty_bind_interface(handle, IOTCON_INTERFACE_GROUP);

	return handle;
}


extern "C" int icd_ioty_unregister_resource(iotcon_resource_h resource_handle)
{
	OCResourceHandle resourceHandle = resource_handle;

	OCStackResult result = unregisterResource(resourceHandle);
	if (OC_STACK_OK != result) {
		ERR("unregisterResource Fail(%d)", result);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


static int _icd_ioty_convert_interface_flag(iotcon_interface_e src, string &dest)
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
		ERR("Invalid interface(%d)", src);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}
	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_bind_interface(OCResourceHandle resourceHandle,
		iotcon_interface_e iface)
{
	int ret;
	OCStackResult ocRet;
	string resource_interface;

	ret = _icd_ioty_convert_interface_flag(iface, resource_interface);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_icd_ioty_convert_interface_flag(%d) Fail(%d)", iface, ret);
		return ret;
	}

	ocRet = bindInterfaceToResource(resourceHandle, resource_interface);
	if (OC_STACK_OK != ocRet) {
		ERR("bindInterfaceToResource() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_bind_type(OCResourceHandle resource_handle,
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


extern "C" int icd_ioty_bind_resource(OCResourceHandle parent, OCResourceHandle child)
{
	OCStackResult ret;

	ret = bindResource(parent, child);
	if (OC_STACK_OK != ret) {
		ERR("bindResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_unbind_resource(OCResourceHandle parent, OCResourceHandle child)
{
	OCStackResult ret;

	ret = unbindResource(parent, child);
	if (OC_STACK_OK != ret) {
		ERR("unbindResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_notify_list_of_observers(int resHandle, GVariant *msg,
		GVariant *observers)
{
	int ret;
	int error_code;
	int obs_id;
	char *repr_json = NULL;
	iotcon_interface_e iface;
	GVariantIter *msgIter;
	GVariantIter *obsIter;
	ObservationIds obsIds;
	string iface_str;
	OCStackResult ocRet;
	OCRepresentation ocRep;

	g_variant_get(observers, "ai", &obsIter);
	while (g_variant_iter_loop(obsIter, "i", &obs_id))
		obsIds.push_back(obs_id);
	g_variant_iter_free(obsIter);

	shared_ptr<OCResourceResponse> resourceResponse(new OCResourceResponse());

	g_variant_get(msg, "a(iis)", &msgIter);
	if (g_variant_iter_loop(msgIter, "(ii&s)", &error_code, &iface, &repr_json)) {
		resourceResponse->setErrorCode(error_code);

		if (IOTCON_INTERFACE_NONE != iface) {
			ret = _icd_ioty_convert_interface_flag(iface, iface_str);
			if (IOTCON_ERROR_NONE != ret) {
				ERR("_icd_ioty_convert_interface_flag(%d) Fail(%d)", iface, ret);
				g_variant_iter_free(msgIter);
				return ret;
			}
		} else {
			iface_str = DEFAULT_INTERFACE;
		}

		ret = icd_ioty_repr_parse_json(repr_json, ocRep);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icd_ioty_repr_parse_json() Fail(%d)", ret);
			g_variant_iter_free(msgIter);
			return ret;
		}

		resourceResponse->setResourceRepresentation(ocRep, iface_str);
	}
	g_variant_iter_free(msgIter);

	ocRet = notifyListOfObservers(GINT_TO_POINTER(resHandle), obsIds, resourceResponse);
	if (OC_STACK_NO_OBSERVERS == ocRet) {
		WARN("No observers. Stop notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ocRet) {
		ERR("notifyListOfObservers() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_notify_all(int resHandle)
{
	OCStackResult ocRet;

	ocRet = notifyAllObservers(GINT_TO_POINTER(resHandle));
	if (OC_STACK_NO_OBSERVERS == ocRet) {
		WARN("No observers. Stop notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ocRet) {
		ERR("notifyAllObservers() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_send_response(GVariant *resp)
{
	FN_CALL;
	int ret;
	int result;
	int error_code;
	int request_handle;
	int resource_handle;
	unsigned short option_id;
	char *option_data;
	char *new_uri;
	const char *repr_json;
	GVariantIter *options;
	iotcon_interface_e iface;
	string iface_str;
	HeaderOptions header_options;
	OCStackResult ocRet;
	OCRepresentation ocRep;

	g_variant_get(resp, "(&sia(qs)ii&sii)",
			&new_uri,
			&error_code,
			&options,
			&iface,
			&result,
			&repr_json,
			&request_handle,
			&resource_handle);

	auto pResponse = make_shared<OCResourceResponse>();
	if (NULL == pResponse) {
		ERR("NULL == pResponse");
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	pResponse->setRequestHandle(GINT_TO_POINTER(request_handle));
	pResponse->setResourceHandle(GINT_TO_POINTER(resource_handle));
	pResponse->setErrorCode(error_code);
	pResponse->setResponseResult((OCEntityHandlerResult)result);

	if (IC_STR_EQUAL != strcmp(new_uri, IC_STR_NULL))
		pResponse->setNewResourceUri(new_uri);

	if (IOTCON_INTERFACE_NONE != iface) {
		ret = _icd_ioty_convert_interface_flag(iface, iface_str);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_icd_ioty_convert_interface_flag(%d) Fail(%d)", iface, ret);
			return ret;
		}
	} else {
		iface_str = DEFAULT_INTERFACE;
	}

	ret = icd_ioty_repr_parse_json(repr_json, ocRep);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_repr_parse_json() Fail(%d)", ret);
		return ret;
	}

	pResponse->setResourceRepresentation(ocRep, iface_str);

	while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data)) {
		HeaderOption::OCHeaderOption option(option_id, option_data);
		header_options.push_back(option);
	}
	g_variant_iter_free(options);

	pResponse->setHeaderOptions(header_options);

	ocRet = sendResponse(pResponse);
	if (OC_STACK_OK != ocRet) {
		ERR("sendResponse() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}



extern "C" int icd_ioty_find_resource(const char *host_address, const char *resource_type,
		const char *sig_name)
{
	FN_CALL;
	OCStackResult ret;
	ostringstream resource_name;

	resource_name << ICD_COAP;

	if (IC_STR_EQUAL == strcmp(IOTCON_MULTICAST_ADDRESS, host_address))
		resource_name << host_address << ICD_MULTICAST_RESOURCE_DISCOVERY;
	else
		resource_name << host_address << ICD_UNICAST_RESOURCE_DISCOVERY;

	if (resource_type)
		resource_name << "?rt=" << resource_type;

	shared_ptr<icdIotivityHandler::findObject> object
		= make_shared<icdIotivityHandler::findObject>(sig_name);
	FindCallback findCallback = bind(&icdIotivityHandler::findObject::foundResource,
			object, placeholders::_1);

	ret = findResource("", resource_name.str(), findCallback);
	if (OC_STACK_OK != ret) {
		ERR("findResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


static OCResource::Ptr _icd_ioty_create_oc_resource(GVariant *client)
{
	int ifaces;
	int observe_handle;
	int is_observable;
	unsigned short option_id;
	char *type;
	char *host;
	char *uri;
	char *option_data;
	GVariantIter *options;
	GVariantIter *types;

	string resource_host;
	vector<string> resource_types;
	vector<string> resource_ifs;
	HeaderOptions header_options;

	g_variant_get(client, "(&s&sba(qs)asii)", &uri, &host, &is_observable, &options,
			&types, &ifaces, &observe_handle);

	resource_host = string(ICD_COAP) + string(host);

	while (g_variant_iter_loop(options, "(q&s)", &option_id, &option_data)) {
		HeaderOption::OCHeaderOption option(option_id, option_data);
		header_options.push_back(option);
	}
	g_variant_iter_free(options);

	while (g_variant_iter_loop(types, "&s", &type))
		resource_types.push_back(type);
	g_variant_iter_free(types);

	if (IOTCON_INTERFACE_NONE == ifaces) {
		resource_ifs.push_back(DEFAULT_INTERFACE);
	} else {
		if (IOTCON_INTERFACE_DEFAULT & ifaces)
			resource_ifs.push_back(DEFAULT_INTERFACE);

		if (IOTCON_INTERFACE_LINK & ifaces)
			resource_ifs.push_back(LINK_INTERFACE);

		if (IOTCON_INTERFACE_BATCH & ifaces)
			resource_ifs.push_back(BATCH_INTERFACE);

		if (IOTCON_INTERFACE_GROUP & ifaces)
			resource_ifs.push_back(GROUP_INTERFACE);
	}

	OCResource::Ptr ocResource = constructResourceObject(resource_host, uri,
			is_observable, resource_types, resource_ifs);

	ocResource->setHeaderOptions(header_options);

	return ocResource;
}


extern "C" int icd_ioty_get(GVariant *resource, GVariant *query, const char *sig_name)
{
	FN_CALL;
	OCStackResult ret;
	OCResource::Ptr ocResource;
	QueryParamsMap queryParams;
	GVariantIter *queryIter;
	char *key;
	char *value;

	g_variant_get(query, "a(ss)", &queryIter);
	while (g_variant_iter_loop(queryIter, "(&s&s)", &key, &value))
		queryParams[key] = value;
	g_variant_iter_free(queryIter);

	ocResource = _icd_ioty_create_oc_resource(resource);

	shared_ptr<icdIotivityHandler::getObject> object
		= make_shared<icdIotivityHandler::getObject>(sig_name);
	GetCallback getCallback = bind(&icdIotivityHandler::getObject::onGet, object,
			placeholders::_1, placeholders::_2, placeholders::_3);
	ret = ocResource->get(queryParams, getCallback);
	if (OC_STACK_OK != ret) {
		ERR("get() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_put(GVariant *resource, const char *repr, GVariant *query,
		const char *sig_name)
{
	FN_CALL;
	int ret;
	OCStackResult ocRet;
	OCResource::Ptr ocResource;
	QueryParamsMap queryParams;
	OCRepresentation ocRep;
	GVariantIter *queryIter;
	char *key;
	char *value;

	g_variant_get(query, "a(ss)", &queryIter);
	while (g_variant_iter_loop(queryIter, "(&s&s)", &key, &value))
		queryParams[key] = value;
	g_variant_iter_free(queryIter);

	ret = icd_ioty_repr_parse_json(repr, ocRep);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_repr_parse_json() Fail(%d)", ret);
		return ret;
	}

	ocResource = _icd_ioty_create_oc_resource(resource);

	shared_ptr<icdIotivityHandler::putObject> object
		= make_shared<icdIotivityHandler::putObject>(sig_name);
	PutCallback putCallback = bind(&icdIotivityHandler::putObject::onPut, object,
			placeholders::_1, placeholders::_2, placeholders::_3);

	ocRet = ocResource->put(ocRep, queryParams, putCallback);
	if (OC_STACK_OK != ocRet) {
		ERR("put() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_post(GVariant *resource, const char *repr, GVariant *query,
		const char *sig_name)
{
	FN_CALL;
	int ret;
	OCStackResult ocRet;
	OCResource::Ptr ocResource;
	QueryParamsMap queryParams;
	OCRepresentation ocRep;
	GVariantIter *queryIter;
	char *key;
	char *value;

	g_variant_get(query, "a(ss)", &queryIter);
	while (g_variant_iter_loop(queryIter, "(&s&s)", &key, &value))
		queryParams[key] = value;
	g_variant_iter_free(queryIter);

	ret = icd_ioty_repr_parse_json(repr, ocRep);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_repr_parse_json() Fail(%d)", ret);
		return ret;
	}

	ocResource = _icd_ioty_create_oc_resource(resource);

	shared_ptr<icdIotivityHandler::postObject> object
		= make_shared<icdIotivityHandler::postObject>(sig_name);
	PostCallback postCallback = bind(&icdIotivityHandler::postObject::onPost, object,
			placeholders::_1, placeholders::_2, placeholders::_3);

	ocRet = ocResource->post(ocRep, queryParams, postCallback);
	if (OC_STACK_OK != ocRet) {
		ERR("post() Fail(%d)", ocRet);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_delete(GVariant *resource, const char *sig_name)
{
	FN_CALL;
	OCStackResult ret;
	OCResource::Ptr ocResource;

	ocResource = _icd_ioty_create_oc_resource(resource);

	shared_ptr<icdIotivityHandler::deleteObject> object
		= make_shared<icdIotivityHandler::deleteObject>(sig_name);
	DeleteCallback deleteCallback = bind(&icdIotivityHandler::deleteObject::onDelete,
			object, placeholders::_1, placeholders::_2);

	ret = ocResource->deleteResource(deleteCallback);
	if (OC_STACK_OK != ret) {
		ERR("deleteResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_observer_start(GVariant *resource, int observe_type, GVariant *query,
		const char *sig_name, int *observe_h)
{
	OCStackResult ret;
	OCResource::Ptr ocResource;
	ObserveType observeType;
	QueryParamsMap queryParams;
	OCRepresentation ocRep;
	GVariantIter *queryIter;
	char *key;
	char *value;

	g_variant_get(query, "a(ss)", &queryIter);
	while (g_variant_iter_loop(queryIter, "(&s&s)", &key, &value))
		queryParams[key] = value;
	g_variant_iter_free(queryIter);

	if (IOTCON_OBSERVE == observe_type) {
		observeType = ObserveType::Observe;
	} else if (IOTCON_OBSERVE_ALL == observe_type) {
		observeType = ObserveType::ObserveAll;
	} else {
		ERR("Invalid observe_type");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	ocResource = _icd_ioty_create_oc_resource(resource);

	resource_handle *obs_h = new resource_handle();
	obs_h->ocResource = ocResource;
	*observe_h = GPOINTER_TO_INT((void*)obs_h);

	shared_ptr<icdIotivityHandler::observeObject> object
		= make_shared<icdIotivityHandler::observeObject>(sig_name);
	ObserveCallback observeCallback = bind(&icdIotivityHandler::observeObject::onObserve,
			object, placeholders::_1, placeholders::_2, placeholders::_3,
			placeholders::_4);
	ret = ocResource->observe(observeType, queryParams, observeCallback);
	if (OC_STACK_OK != ret) {
		ERR("observe() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_observer_stop(void *observe_h)
{
	OCStackResult ret;

	resource_handle *resource_h = (resource_handle*)observe_h;

	OCResource::Ptr ocResource = resource_h->ocResource;
	delete (resource_handle*)observe_h;

	ret = ocResource->cancelObserve(QualityOfService::HighQos);
	if (OC_STACK_OK != ret) {
		ERR("cancelObserve() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_register_device_info(GVariant *value)
{
	FN_CALL;
	OCStackResult ret;

	OCDeviceInfo info = {0};

	g_variant_get(value, "(&s&s&s&s&s&s&s&s&s&s&s&s)",
			&info.deviceName,
			&info.hostName,
			&info.deviceUUID,
			&info.contentType,
			&info.version,
			&info.manufacturerName,
			&info.manufacturerUrl,
			&info.modelNumber,
			&info.dateOfManufacture,
			&info.platformVersion,
			&info.firmwareVersion,
			&info.supportUrl);

	ret = registerDeviceInfo(info);
	if (OC_STACK_OK != ret) {
		ERR("registerDeviceInfo() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_get_device_info(const char *host_address,
		const char *sig_name)
{
	FN_CALL;
	OCStackResult ret;
	string resHost = string(ICD_COAP) + host_address + string(ICD_DEVICE_DISCOVERY);

	shared_ptr<icdIotivityHandler::deviceObject> object
		= make_shared<icdIotivityHandler::deviceObject>(sig_name);
	FindDeviceCallback findDeviceCallback = bind(
			&icdIotivityHandler::deviceObject::receivedDeviceInfo,
			object,
			placeholders::_1);

	ret = getDeviceInfo("", resHost, findDeviceCallback);
	if (OC_STACK_OK != ret) {
		ERR("getDeviceInfo() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" iotcon_presence_h icd_ioty_subscribe_presence(const char *host_address,
		const char *resource_type,
		const char *sig_name)
{
	OCStackResult ret;
	iotcon_presence_h presence_handle = NULL;
	string host;

	shared_ptr<icdIotivityHandler::presenceObject> object
		= make_shared<icdIotivityHandler::presenceObject>(sig_name);
	SubscribeCallback subscribeCallback
		= bind(&icdIotivityHandler::presenceObject::presenceHandler, object,
				placeholders::_1, placeholders::_2, placeholders::_3);

	host = string(ICD_COAP) + string(host_address);
	ret = subscribePresence(presence_handle, host, resource_type, subscribeCallback);

	if (OC_STACK_OK != ret) {
		ERR("subscribePresence() Fail(%d)", ret);
		return NULL;
	}

	return presence_handle;
}


extern "C" int icd_ioty_unsubscribe_presence(iotcon_presence_h presence_handle)
{
	OCStackResult ret;

	ret = unsubscribePresence(presence_handle);
	if (OC_STACK_OK != ret) {
		ERR("unsubscribePresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_start_presence(unsigned int time_to_live)
{
	OCStackResult ret;

	ret = startPresence(time_to_live);
	if (OC_STACK_OK != ret) {
		ERR("startPresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


extern "C" int icd_ioty_stop_presence()
{
	OCStackResult ret;

	ret = stopPresence();
	if (OC_STACK_OK != ret) {
		ERR("stopPresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


