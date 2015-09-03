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
#include <stdint.h> /* for uint8_t etc */
#include <stdbool.h>
#include <errno.h>
#include <glib.h>

#include <octypes.h>
#include <ocstack.h>

#include "iotcon.h"
#include "ic-dbus.h"
#include "ic-utils.h"
#include "icd.h"
#include "icd-payload.h"
#include "icd-dbus.h"
#include "icd-ioty.h"
#include "icd-ioty-type.h"
#include "icd-ioty-ocprocess.h"

static GMutex icd_csdk_mutex;

void icd_ioty_csdk_lock()
{
	g_mutex_lock(&icd_csdk_mutex);
}


void icd_ioty_csdk_unlock()
{
	g_mutex_unlock(&icd_csdk_mutex);
}


GThread* icd_ioty_init(const char *addr, unsigned short port)
{
	FN_CALL;
	GError *error;
	GThread *thread;

	OCStackResult result = OCInit(addr, port, OC_CLIENT_SERVER);
	if (OC_STACK_OK != result) {
		ERR("OCInit() Fail(%d)", result);
		return NULL;
	}

	DBG("OCInit() Success");

	thread = g_thread_try_new("packet_receive_thread", icd_ioty_ocprocess_thread,
			NULL, &error);
	if (NULL == thread) {
		ERR("g_thread_try_new() Fail(%s)", error->message);
		g_error_free(error);
		return NULL;
	}

	return thread;
}


void icd_ioty_deinit(GThread *thread)
{
	OCStackResult result;

	icd_ioty_ocprocess_stop();
	g_thread_join(thread);

	result = OCStop();
	if (OC_STACK_OK != result)
		ERR("OCStop() Fail(%d)", result);
}


OCResourceHandle icd_ioty_register_resource(const char *uri_path,
		const char* const* res_types, int ifaces, uint8_t properties)
{
	FN_CALL;
	int i;
	OCStackResult ret;
	OCResourceHandle handle;
	const char *res_iface = NULL;

	if (IOTCON_INTERFACE_DEFAULT & ifaces) {
		res_iface = IC_INTERFACE_DEFAULT;
		ifaces ^= IOTCON_INTERFACE_DEFAULT;
	} else if (IOTCON_INTERFACE_LINK & ifaces) {
		res_iface = IC_INTERFACE_LINK;
		ifaces ^= IOTCON_INTERFACE_LINK;
	} else if (IOTCON_INTERFACE_BATCH & ifaces) {
		res_iface = IC_INTERFACE_BATCH;
		ifaces ^= IOTCON_INTERFACE_BATCH;
	} else if (IOTCON_INTERFACE_GROUP & ifaces) {
		res_iface = IC_INTERFACE_GROUP;
		ifaces ^= IOTCON_INTERFACE_GROUP;
	} else {
		ERR("Invalid interface type(%d)", ifaces);
		return NULL;
	}

	/* Secure option is not supported yet. */
	properties = (properties & OC_SECURE)? (properties ^ OC_SECURE):properties;

	icd_ioty_csdk_lock();
	ret = OCCreateResource(&handle, res_types[0], res_iface, uri_path,
			icd_ioty_ocprocess_req_handler, NULL, properties);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCCreateResource() Fail(%d)", ret);
		return NULL;
	}

	for (i = 1; res_types[i]; i++)
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


int icd_ioty_unregister_resource(OCResourceHandle handle)
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	ret = OCDeleteResource(handle);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCDeleteResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_bind_interface(OCResourceHandle handle, iotcon_interface_e iface)
{
	int ret;
	OCStackResult result;
	char *resource_interface;

	ret = ic_utils_convert_interface_flag(iface, &resource_interface);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("ic_utils_convert_interface_flag(%d) Fail(%d)", iface, ret);
		return ret;
	}

	icd_ioty_csdk_lock();
	result = OCBindResourceInterfaceToResource(handle, resource_interface);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != result) {
		ERR("OCBindResourceInterfaceToResource() Fail(%d)", result);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_bind_type(OCResourceHandle handle, const char *resource_type)
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	ret = OCBindResourceTypeToResource(handle, resource_type);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCBindResourceTypeToResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_bind_resource(OCResourceHandle parent, OCResourceHandle child)
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	ret = OCBindResource(parent, child);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCBindResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_unbind_resource(OCResourceHandle parent, OCResourceHandle child)
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	ret = OCUnBindResource(parent, child);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCUnBindResource() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_notify_list_of_observers(OCResourceHandle handle, GVariant *msg,
		GVariant *observers)
{
	int i, error_code, obs_length;
	GVariant *repr_gvar;
	GVariantIter obs_iter, msg_iter;
	OCStackResult ret;
	OCRepPayload *payload;

	g_variant_iter_init(&obs_iter, observers);
	obs_length = g_variant_iter_n_children(&obs_iter);

	/* Variable-length Array */
	OCObservationId obs_ids[obs_length];

	for (i = 0; i < obs_length; i++)
		g_variant_iter_loop(&obs_iter, "i", &obs_ids[i]);

	g_variant_iter_init(&msg_iter, msg);
	g_variant_iter_loop(&msg_iter, "(iv)", &error_code, &repr_gvar);
	/* TODO : How to use error_code. */

	payload = icd_payload_representation_from_gvariant(repr_gvar);

	icd_ioty_csdk_lock();
	/* TODO : QoS is come from lib. */
	ret = OCNotifyListOfObservers(handle, obs_ids, obs_length, payload, OC_LOW_QOS);
	icd_ioty_csdk_unlock();

	if (OC_STACK_NO_OBSERVERS == ret) {
		WARN("No Observers. Stop Notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ret) {
		ERR("OCNotifyListOfObservers() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_notify_all(OCResourceHandle handle)
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	/* TODO : QoS is come from lib. */
	ret = OCNotifyAllObservers(handle, OC_LOW_QOS);
	icd_ioty_csdk_unlock();

	if (OC_STACK_NO_OBSERVERS == ret) {
		WARN("No Observers. Stop Notifying");
		return IOTCON_ERROR_NONE;
	} else if (OC_STACK_OK != ret) {
		ERR("OCNotifyAllObservers() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


static int _ioty_get_header_options(GVariantIter *src, int src_size,
		OCHeaderOption dest[], int dest_size)
{
	int i = 0;
	char *option_data;
	unsigned short option_id;

	RETV_IF(NULL == dest, IOTCON_ERROR_INVALID_PARAMETER);

	if (dest_size < src_size) {
		ERR("Exceed Size(%d)", src_size);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	while (g_variant_iter_loop(src, "(q&s)", &option_id, &option_data)) {
		dest[i].protocolID = OC_COAP_ID;
		dest[i].optionID = option_id;
		dest[i].optionLength = strlen(option_data) + 1;
		memcpy(dest[i].optionData, option_data, dest[i].optionLength);
		i++;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_send_response(GVariant *resp)
{
	char *new_uri_path;
	GVariant *repr_gvar;
	GVariantIter *options;
	OCStackResult ret;
	OCEntityHandlerResponse response = {0};
	int result, error_code, options_size;
	int64_t request_handle, resource_handle;

	g_variant_get(resp, "(&sia(qs)ivxx)",
			&new_uri_path,
			&error_code,
			&options,
			&result,
			&repr_gvar,
			&request_handle,
			&resource_handle);

	response.requestHandle = ICD_INT64_TO_POINTER(request_handle);
	response.resourceHandle = ICD_INT64_TO_POINTER(resource_handle);
	response.ehResult = (OCEntityHandlerResult)result;

	if (OC_EH_RESOURCE_CREATED == response.ehResult)
		snprintf(response.resourceUri, sizeof(response.resourceUri), "%s", new_uri_path);

	options_size = g_variant_iter_n_children(options);
	response.numSendVendorSpecificHeaderOptions = options_size;

	if (0 != options_size) {
		int ret= _ioty_get_header_options(options,
				response.numSendVendorSpecificHeaderOptions,
				response.sendVendorSpecificHeaderOptions,
				sizeof(response.sendVendorSpecificHeaderOptions)
				/ sizeof(response.sendVendorSpecificHeaderOptions[0]));

		if (IOTCON_ERROR_NONE != ret)
			ERR("_ioty_get_header_options() Fail(%d)", ret);
	}
	g_variant_iter_free(options);

	response.payload = (OCPayload*)icd_payload_representation_from_gvariant(repr_gvar);

	/* related to block transfer */
	response.persistentBufferFlag = 0;

	icd_ioty_csdk_lock();
	ret = OCDoResponse(&response);
	icd_ioty_csdk_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCDoResponse() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


static void _ioty_free_signal_context(void *data)
{
	icd_sig_ctx_s *context = data;
	free(context->bus_name);
	free(context);
}


int icd_ioty_find_resource(const char *host_address, const char *resource_type,
		unsigned int signum, const char *bus_name)
{
	int len;
	OCStackResult result;
	icd_sig_ctx_s *context;
	char uri[PATH_MAX] = {0};
	OCCallbackData cbdata = {0};
	OCConnectivityType oic_conn_type;
	iotcon_connectivity_type_e conn_type = IOTCON_CONNECTIVITY_IPV4;

	if (IC_STR_EQUAL == strcmp(IOTCON_MULTICAST_ADDRESS, host_address)) {
		len = snprintf(uri, sizeof(uri), "%s", OC_RSRVD_WELL_KNOWN_URI);
	} else {
		len = snprintf(uri, sizeof(uri), ICD_IOTY_COAP"%s%s", host_address,
				OC_RSRVD_WELL_KNOWN_URI);
	}
	if (len <= 0 || sizeof(uri) <= len) {
		ERR("snprintf() Fail(%d)", len);
		return IOTCON_ERROR_IO_ERROR;
	}

	if (IC_STR_EQUAL != strcmp(IC_STR_NULL, resource_type))
		snprintf(uri + len, sizeof(uri), "?rt=%s", resource_type);

	context = calloc(1, sizeof(icd_sig_ctx_s));
	if (NULL == context) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	context->bus_name = ic_utils_strdup(bus_name);
	context->signum = signum;

	cbdata.context = context;
	cbdata.cb = icd_ioty_ocprocess_find_cb;
	cbdata.cd = _ioty_free_signal_context;

	oic_conn_type = icd_ioty_conn_type_to_oic_conn_type(conn_type);

	icd_ioty_csdk_lock();
	/* TODO : QoS is come from lib. */
	result = OCDoResource(NULL, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icd_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		free(context->bus_name);
		free(context);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


/*
 * returned string SHOULD be released by you
 */
static char* _icd_ioty_resource_generate_uri(char *uri_path, GVariant *query)
{
	int len;
	bool loop_first = true;
	char *key, *value;
	GVariantIter query_iter;
	char uri_buf[PATH_MAX] = {0};

	len = snprintf(uri_buf, sizeof(uri_buf), "%s", uri_path);

	/* remove suffix '/' */
	if ('/' == uri_buf[strlen(uri_buf) - 1]) {
		uri_buf[strlen(uri_buf) - 1] = '\0';
		len--;
	}

	g_variant_iter_init(&query_iter, query);

	while (g_variant_iter_loop(&query_iter, "(&s&s)", &key, &value)) {
		int query_len;

		DBG("query exist. key(%s), value(%s)", key, value);

		if (true == loop_first) {
			query_len = snprintf(uri_buf + len, sizeof(uri_buf), "?%s=%s", key, value);
			loop_first = false;
		} else {
			query_len = snprintf(uri_buf + len, sizeof(uri_buf), "&%s=%s", key, value);
		}

		len += query_len;
	}

	return strdup(uri_buf);
}


void icd_ioty_complete(int type, GDBusMethodInvocation *invocation, GVariant *value)
{
	switch(type) {
	case ICD_CRUD_GET:
		ic_dbus_complete_get(icd_dbus_get_object(), invocation, value);
		break;
	case ICD_CRUD_PUT:
		ic_dbus_complete_put(icd_dbus_get_object(), invocation, value);
		break;
	case ICD_CRUD_POST:
		ic_dbus_complete_post(icd_dbus_get_object(), invocation, value);
		break;
	case ICD_CRUD_DELETE:
		ic_dbus_complete_delete(icd_dbus_get_object(), invocation, value);
		break;
	}
}


void icd_ioty_complete_error(int type, GDBusMethodInvocation *invocation, int ret_val)
{
	GVariant *value;

	switch (type) {
	case ICD_CRUD_GET:
		value = g_variant_new("(a(qs)vi)", NULL, NULL, ret_val);
		ic_dbus_complete_get(icd_dbus_get_object(), invocation, value);
		break;
	case ICD_CRUD_PUT:
		value = g_variant_new("(a(qs)vi)", NULL, NULL, ret_val);
		ic_dbus_complete_put(icd_dbus_get_object(), invocation, value);
		break;
	case ICD_CRUD_POST:
		value = g_variant_new("(a(qs)vi)", NULL, NULL, ret_val);
		ic_dbus_complete_post(icd_dbus_get_object(), invocation, value);
		break;
	case ICD_CRUD_DELETE:
		value = g_variant_new("(a(qs)i)", NULL, ret_val);
		ic_dbus_complete_delete(icd_dbus_get_object(), invocation, value);
		break;
	}
}


static gboolean _icd_ioty_crud(int type, icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *query, GVariant *repr)
{
	bool is_secure;
	OCMethod rest_type;
	OCStackResult result;
	GVariantIter *options;
	OCCallbackData cbdata = {0};
	int conn_type, options_size;
	char *uri_path, *host, *uri, *dev_host, *ptr;
	OCHeaderOption oic_options[MAX_HEADER_OPTIONS];
	OCHeaderOption *oic_options_ptr = NULL;
	OCPayload *payload = NULL;
	OCConnectivityType oic_conn_type;
	OCDevAddr dev_addr = {0};

	switch (type) {
	case ICD_CRUD_GET:
		cbdata.cb = icd_ioty_ocprocess_get_cb;
		rest_type = OC_REST_GET;
		break;
	case ICD_CRUD_PUT:
		cbdata.cb = icd_ioty_ocprocess_put_cb;
		rest_type = OC_REST_PUT;
		break;
	case ICD_CRUD_POST:
		cbdata.cb = icd_ioty_ocprocess_post_cb;
		rest_type = OC_REST_POST;
		break;
	case ICD_CRUD_DELETE:
		cbdata.cb = icd_ioty_ocprocess_delete_cb;
		rest_type = OC_REST_DELETE;
		break;
	default:
		ERR("Invalid CRUD Type(%d)", type);
		return FALSE;
	}

	g_variant_get(resource, "(&s&sba(qs)i)", &uri_path, &host, &is_secure, &options,
			&conn_type);

	switch (type) {
	case ICD_CRUD_GET:
	case ICD_CRUD_PUT:
	case ICD_CRUD_POST:
		uri = _icd_ioty_resource_generate_uri(uri_path, query);
		if (NULL == uri) {
			ERR("_icd_ioty_resource_generate_uri() Fail");
			g_variant_iter_free(options);
			icd_ioty_complete_error(type, invocation, IOTCON_ERROR_INVALID_PARAMETER);
			return TRUE;
		}
		break;
	case ICD_CRUD_DELETE:
		uri = strdup(uri_path);
		break;
	}

	cbdata.context = invocation;

	options_size = g_variant_iter_n_children(options);
	if (0 != options_size) {
		int ret = _ioty_get_header_options(options, options_size, oic_options,
				sizeof(oic_options) / sizeof(oic_options[0]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_ioty_get_header_options() Fail(%d)", ret);
			free(uri);
			g_variant_iter_free(options);
			icd_ioty_complete_error(type, invocation, ret);
			return TRUE;
		}
		oic_options_ptr = oic_options;
	}
	g_variant_iter_free(options);

	if (repr)
		payload = (OCPayload*)icd_payload_representation_from_gvariant(repr);

	oic_conn_type = icd_ioty_conn_type_to_oic_conn_type(conn_type);

	icd_ioty_conn_type_to_oic_transport_type(conn_type, &dev_addr.adapter,
			&dev_addr.flags);

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		dev_host = strtok_r(host, ":", &ptr);
		snprintf(dev_addr.addr, sizeof(dev_addr.addr), "%s", dev_host);
		dev_addr.port = atoi(strtok_r(NULL, ":", &ptr));
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		dev_host = strtok_r(host, "]", &ptr);
		snprintf(dev_addr.addr, sizeof(dev_addr.addr), "%s", dev_host);
		dev_addr.port = atoi(strtok_r(NULL, "]", &ptr));
		break;
	default:
		ERR("Invalid Connectivitiy Type");
		icd_ioty_complete_error(type, invocation, IOTCON_ERROR_IOTIVITY);
		return TRUE;
	}

	icd_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	result = OCDoResource(NULL, rest_type, uri, &dev_addr, payload, oic_conn_type,
			OC_LOW_QOS, &cbdata, oic_options_ptr, options_size);
	icd_ioty_csdk_unlock();

	free(uri);

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		icd_ioty_complete_error(type, invocation, IOTCON_ERROR_IOTIVITY);
		return TRUE;
	}

	return TRUE;
}

gboolean icd_ioty_get(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *query)
{
	return _icd_ioty_crud(ICD_CRUD_GET, object, invocation, resource, query, NULL);
}


gboolean icd_ioty_put(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *repr, GVariant *query)
{
	return _icd_ioty_crud(ICD_CRUD_PUT, object, invocation, resource, query, repr);
}


gboolean icd_ioty_post(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource, GVariant *repr, GVariant *query)
{
	return _icd_ioty_crud(ICD_CRUD_POST, object, invocation, resource, query, repr);
}


gboolean icd_ioty_delete(icDbus *object, GDBusMethodInvocation *invocation,
		GVariant *resource)
{
	return _icd_ioty_crud(ICD_CRUD_DELETE, object, invocation, resource, NULL, NULL);
}


OCDoHandle icd_ioty_observer_start(GVariant *resource, int observe_type, GVariant *query,
		unsigned int signal_number, const char *bus_name)
{
	bool is_secure;
	OCMethod method;
	OCDoHandle handle;
	OCStackResult result;
	GVariantIter *options;
	icd_sig_ctx_s *context;
	OCCallbackData cbdata = {0};
	int conn_type, options_size;
	char *uri_path, *host, *uri, *dev_host, *ptr;
	OCHeaderOption oic_options[MAX_HEADER_OPTIONS];
	OCHeaderOption *oic_options_ptr = NULL;
	OCConnectivityType oic_conn_type;
	OCDevAddr dev_addr = {0};

	g_variant_get(resource, "(&s&sba(qs)i)", &uri_path, &host, &is_secure,  &options,
			&conn_type);

	uri = _icd_ioty_resource_generate_uri(uri_path, query);
	if (NULL == uri) {
		ERR("_icd_ioty_resource_generate_uri() Fail");
		g_variant_iter_free(options);
		return NULL;
	}

	if (IOTCON_OBSERVE == observe_type)
		method = OC_REST_OBSERVE;
	else if (IOTCON_OBSERVE_ALL == observe_type)
		method = OC_REST_OBSERVE_ALL;
	else
		method = OC_REST_OBSERVE_ALL;

	context = calloc(1, sizeof(icd_sig_ctx_s));
	if (NULL == context) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}
	context->bus_name = ic_utils_strdup(bus_name);
	context->signum = signal_number;

	cbdata.context = context;
	cbdata.cb = icd_ioty_ocprocess_observe_cb;
	cbdata.cd = _ioty_free_signal_context;

	options_size = g_variant_iter_n_children(options);
	if (0 != options_size) {
		int ret = _ioty_get_header_options(options, options_size, oic_options,
				sizeof(oic_options) / sizeof(oic_options[0]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_ioty_get_header_options() Fail(%d)", ret);
			free(context->bus_name);
			free(context);
			free(uri);
			g_variant_iter_free(options);
			return NULL;
		}
		oic_options_ptr = oic_options;
	}
	g_variant_iter_free(options);

	oic_conn_type = icd_ioty_conn_type_to_oic_conn_type(conn_type);

	icd_ioty_conn_type_to_oic_transport_type(conn_type, &dev_addr.adapter,
			&dev_addr.flags);

	switch (conn_type) {
	case IOTCON_CONNECTIVITY_IPV4:
		dev_host = strtok_r(host, ":", &ptr);
		snprintf(dev_addr.addr, sizeof(dev_addr.addr), "%s", dev_host);
		dev_addr.port = atoi(strtok_r(NULL, ":", &ptr));
		break;
	case IOTCON_CONNECTIVITY_IPV6:
		dev_host = strtok_r(host, "]", &ptr);
		snprintf(dev_addr.addr, sizeof(dev_addr.addr), "%s", dev_host);
		dev_addr.port = atoi(strtok_r(NULL, "]", &ptr));
		break;
	default:
		ERR("Invalid Connectivitiy Type");
		return NULL;
	}

	icd_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	result = OCDoResource(&handle, method, uri, &dev_addr, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, oic_options_ptr, options_size);
	icd_ioty_csdk_unlock();
	free(uri);
	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		free(context->bus_name);
		free(context);
		return NULL;
	}

	return handle;
}


int icd_ioty_observer_stop(OCDoHandle handle, GVariant *options)
{
	int options_size;
	OCStackResult ret;
	GVariantIter options_iter;
	OCHeaderOption oic_options[MAX_HEADER_OPTIONS];
	OCHeaderOption *oic_options_ptr = NULL;

	g_variant_iter_init(&options_iter, options);

	options_size = g_variant_iter_n_children(&options_iter);
	if (0 != options_size) {
		int ret = _ioty_get_header_options(&options_iter, options_size, oic_options,
				sizeof(oic_options) / sizeof(oic_options[0]));
		if (IOTCON_ERROR_NONE != ret) {
			ERR("_ioty_get_header_options() Fail(%d)", ret);
			return ret;
		}
		oic_options_ptr = oic_options;
	}

	icd_ioty_csdk_lock();
	ret = OCCancel(handle, OC_HIGH_QOS, oic_options_ptr, options_size);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCCancel() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_register_device_info(GVariant *value)
{
	OCStackResult result;
	OCDeviceInfo device_info = {0};

	g_variant_get(value, "(&s)", &device_info.deviceName);

	icd_ioty_csdk_lock();
	result = OCSetDeviceInfo(device_info);
	icd_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCSetDeviceInfo() Fail(%d)", result);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_register_platform_info(GVariant *value)
{
	OCStackResult result;
	OCPlatformInfo platform_info = {0};

	g_variant_get(value, "(&s&s&s&s&s&s&s&s&s&s&s)",
			&platform_info.platformID,
			&platform_info.manufacturerName,
			&platform_info.manufacturerUrl,
			&platform_info.modelNumber,
			&platform_info.dateOfManufacture,
			&platform_info.platformVersion,
			&platform_info.operatingSystemVersion,
			&platform_info.hardwareVersion,
			&platform_info.firmwareVersion,
			&platform_info.supportUrl,
			&platform_info.systemTime);

	icd_ioty_csdk_lock();
	result = OCSetPlatformInfo(platform_info);
	icd_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCSetPlatformInfo() Fail(%d)", result);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_get_info(int type, const char *host_address, unsigned int signal_number,
		const char *bus_name)
{
	OCStackResult result;
	icd_sig_ctx_s *context;
	OCCallbackData cbdata = {0};
	char uri[PATH_MAX] = {0};
	char *uri_path = NULL;
	iotcon_connectivity_type_e conn_type = IOTCON_CONNECTIVITY_IPV4;
	OCConnectivityType oic_conn_type;

	if (ICD_DEVICE_INFO == type)
		uri_path = OC_RSRVD_DEVICE_URI;
	else if (ICD_PLATFORM_INFO == type)
		uri_path = OC_RSRVD_PLATFORM_URI;
	else
		return IOTCON_ERROR_INVALID_PARAMETER;

	if (IC_STR_EQUAL == strcmp(IOTCON_MULTICAST_ADDRESS, host_address))
		snprintf(uri, sizeof(uri), "%s", uri_path);
	else
		snprintf(uri, sizeof(uri), "%s%s", host_address, uri_path);

	context = calloc(1, sizeof(icd_sig_ctx_s));
	if (NULL == context) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	context->bus_name = ic_utils_strdup(bus_name);
	context->signum = signal_number;

	cbdata.context = context;
	cbdata.cb = icd_ioty_ocprocess_info_cb;
	cbdata.cd = _ioty_free_signal_context;

	oic_conn_type = icd_ioty_conn_type_to_oic_conn_type(conn_type);

	icd_ioty_csdk_lock();
	/* TODO : QoS is come from lib. And user can set QoS to client structure.  */
	result = OCDoResource(NULL, OC_REST_DISCOVER, uri, NULL, NULL, oic_conn_type,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icd_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		free(context->bus_name);
		free(context);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


OCDoHandle icd_ioty_subscribe_presence(const char *host_address,
		const char *resource_type, unsigned int signal_number, const char *bus_name)
{
	int len;
	OCDoHandle handle;
	OCStackResult result;
	char uri[PATH_MAX] = {0};
	OCCallbackData cbdata = {0};
	icd_sig_ctx_s *context;

	len = snprintf(uri, sizeof(uri), "%s%s", host_address, OC_RSRVD_PRESENCE_URI);
	if (len <= 0 || sizeof(uri) <= len) {
		ERR("snprintf() Fail(%d)", len);
		return NULL;
	}

	if (IC_STR_EQUAL != strcmp(IC_STR_NULL, resource_type))
		snprintf(uri + len, sizeof(uri), "?rt=%s", resource_type);

	context = calloc(1, sizeof(icd_sig_ctx_s));
	if (NULL == context) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}
	context->bus_name = ic_utils_strdup(bus_name);
	context->signum = signal_number;

	cbdata.context = context;
	cbdata.cb = icd_ioty_ocprocess_presence_cb;
	cbdata.cd = _ioty_free_signal_context;

	/* In case of IPV4 or IPV6, connectivity type is CT_ADAPTER_IP in iotivity 0.9.2 */
	icd_ioty_csdk_lock();
	result = OCDoResource(&handle, OC_REST_PRESENCE, uri, NULL, NULL, CT_ADAPTER_IP,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icd_ioty_csdk_unlock();

	if (OC_STACK_OK != result) {
		ERR("OCDoResource() Fail(%d)", result);
		free(context->bus_name);
		free(context);
		return NULL;
	}
	return handle;
}


int icd_ioty_unsubscribe_presence(OCDoHandle handle)
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	ret = OCCancel(handle, OC_LOW_QOS, NULL, 0);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCCancel() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_start_presence(unsigned int time_to_live)
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	ret = OCStartPresence(time_to_live);
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCStartPresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_stop_presence()
{
	OCStackResult ret;

	icd_ioty_csdk_lock();
	ret = OCStopPresence();
	icd_ioty_csdk_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCStopPresence() Fail(%d)", ret);
		return IOTCON_ERROR_IOTIVITY;
	}

	return IOTCON_ERROR_NONE;
}
