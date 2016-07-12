/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <octypes.h>
#include <ocstack.h>
#include <ocprovisioningmanager.h>
#include <oxmjustworks.h>
#include <oxmrandompin.h>
#include <pmutility.h>
#include <doxmresource.h>
#include <verresource.h>

#include "iotcon.h"
#include "iotcon-provisioning.h"
#include "ic-utils.h"
#include "ic-ioty-types.h"
#include "icl.h"
#include "icl-ioty.h"
#include "icl-provisioning-struct.h"

#define ICL_PROVISIONING_TIMEOUT_MAX 10

typedef enum {
	ICL_PROVISIONING_FIND_SECURE_PORT,
	ICL_PROVISIONING_FIND_SECURITY_VERSION,
} icl_provisioning_find_e;

typedef enum {
	ICL_PROVISIONING_MUTEX_FIND,
	ICL_PROVISIONING_MUTEX_INTERNAL_FIND,
} icl_provisioning_mutex_e;

typedef struct {
	int ref_count;
	iotcon_provisioning_found_device_cb cb;
	void *user_data;
	int result;
	int timeout;
	unsigned int timer_id;
	OCDoHandle handle;
	pthread_mutex_t mutex;
} icl_provisioning_find_cb_s;

typedef struct {
	unsigned int timer_id;
	OCDoHandle handle;
	icl_provisioning_find_cb_s *cb_data;
	OCProvisionDev_t *oic_device;
	iotcon_provisioning_device_h device;
	pthread_mutex_t mutex;
} icl_provisioning_find_cb_container_s;

static const char *ICL_PROVISIONING_OWNED_MULTICAST_QUERY = "/oic/sec/doxm?Owned=TRUE";
static const char *ICL_PROVISIONING_UNOWNED_MULTICAST_QUERY = "/oic/sec/doxm?Owned=FALSE";
static const char *ICL_PROVISIONING_ALL_MULTICAST_QUERY = "/oic/sec/doxm";

static const char *ICL_PROVISIONING_DEFAULT_SEC_VERSION = "0.0.0";

static int _provisioning_find_thread(int type, void *user_data);

static inline pthread_mutex_t* _provisioning_find_mutex_get(int type, void *data)
{
	pthread_mutex_t *mutex;
	icl_provisioning_find_cb_s *cb_data;
	icl_provisioning_find_cb_container_s *container;

	switch (type) {
	case ICL_PROVISIONING_MUTEX_FIND:
		cb_data = data;
		mutex = &cb_data->mutex;
		break;
	case ICL_PROVISIONING_MUTEX_INTERNAL_FIND:
		container = data;
		mutex = &container->mutex;
		break;
	default:
		ERR("Invalid Type(%d)", type);
		mutex = NULL;
	}

	return mutex;
}


static void _provisioning_find_mutex_lock(int type, void *data)
{
	int ret;

	ret = pthread_mutex_lock(_provisioning_find_mutex_get(type, data));
	WARN_IF(0 != ret, "pthread_mutex_lock() Fail(%d)", ret);
}


static void _provisioning_find_mutex_unlock(int type, void *data)
{
	int ret;

	ret = pthread_mutex_unlock(_provisioning_find_mutex_get(type, data));
	WARN_IF(0 != ret, "pthread_mutex_unlock() Fail(%d)", ret);
}


static icl_provisioning_find_cb_s* _provisioning_find_cb_ref(
		icl_provisioning_find_cb_s *cb_data)
{
	RETV_IF(NULL == cb_data, NULL);
	RETV_IF(cb_data->ref_count <= 0, NULL);

	cb_data->ref_count++;

	return cb_data;
}


static int _provisioning_find_create_device(const char *addr,
		const uint16_t port,
		OCTransportAdapter adapter,
		OCConnectivityType conn_type,
		OicSecDoxm_t *doxm,
		OCProvisionDev_t **device_list)
{
	OCProvisionDev_t *temp;

	RETV_IF(NULL == addr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == doxm, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == device_list, IOTCON_ERROR_INVALID_PARAMETER);

	temp = calloc(1, sizeof(OCProvisionDev_t));
	if (NULL == temp) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}

	snprintf(temp->endpoint.addr, sizeof(temp->endpoint.addr), "%s", addr);
	temp->endpoint.port = port;
	temp->endpoint.adapter = adapter;
	temp->connType = conn_type;
	temp->doxm = doxm;
	temp->securePort = DEFAULT_SECURE_PORT;
	temp->devStatus = DEV_STATUS_ON;
	snprintf(temp->secVer, sizeof(temp->secVer), "%s",
			ICL_PROVISIONING_DEFAULT_SEC_VERSION);

	*device_list = temp;

	return IOTCON_ERROR_NONE;
}


static gboolean _provisioning_free_cb_data_idle_cb(gpointer p)
{
	icl_provisioning_find_cb_s *cb_data;

	RETV_IF(NULL == p, G_SOURCE_REMOVE);

	cb_data = p;
	cb_data->ref_count--;

	if (0 == cb_data->ref_count)
		free(cb_data);

	return G_SOURCE_REMOVE;
}


static void _provisioning_free_cb_data(void *data)
{
	icl_provisioning_find_cb_s *cb_data = data;
	RET_IF(NULL == cb_data);

	if (cb_data->timer_id) {
		g_source_remove(cb_data->timer_id);
		cb_data->timer_id = 0;
	}
	g_idle_add(_provisioning_free_cb_data_idle_cb, cb_data);
}


static gboolean _provisioning_find_internal_timeout(gpointer user_data)
{
	FN_CALL;
	int ret;
	icl_provisioning_find_cb_container_s *container = user_data;

	RETV_IF(NULL == container, G_SOURCE_REMOVE);

	_provisioning_find_mutex_lock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);
	if (0 == container->timer_id)
		return G_SOURCE_REMOVE;

	container->timer_id = 0;
	_provisioning_find_mutex_unlock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);

	ret = icl_ioty_mutex_lock();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_mutex_lock() Fail(%d)", ret);
		return ret;
	}
	ret = OCCancel(container->handle, OC_LOW_QOS, NULL, 0);
	icl_ioty_mutex_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCCancel() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}

	return G_SOURCE_REMOVE;
}


static gboolean _provisioning_find_timeout(gpointer user_data)
{
	FN_CALL;
	int ret;
	icl_provisioning_find_cb_s *cb_info = user_data;

	RETV_IF(NULL == cb_info, G_SOURCE_REMOVE);

	_provisioning_find_mutex_lock(ICL_PROVISIONING_MUTEX_FIND, cb_info);
	cb_info->timer_id = 0;
	_provisioning_find_mutex_unlock(ICL_PROVISIONING_MUTEX_FIND, cb_info);

	ret = icl_ioty_mutex_lock();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_mutex_lock() Fail(%d)", ret);
		return ret;
	}
	ret = OCCancel(cb_info->handle, OC_LOW_QOS, NULL, 0);
	icl_ioty_mutex_unlock();

	if (OC_STACK_OK != ret) {
		ERR("OCCancel() Fail(%d)", ret);
		return G_SOURCE_REMOVE;
	}

	return G_SOURCE_REMOVE;
}


static void _provisioning_free_find_cb_container(void *data)
{
	icl_provisioning_find_cb_container_s *container = data;

	if (container->device) {
		iotcon_provisioning_device_destroy(container->device);
		container->oic_device = NULL;
	}
	if (container->oic_device)
		OCDeleteDiscoveredDevices(container->oic_device);

	_provisioning_free_cb_data(container->cb_data);

	free(container);
}


static gboolean _provisioning_find_idle_cb(gpointer p)
{
	FN_CALL;
	int ret;
	icl_provisioning_find_cb_container_s *container;
	iotcon_provisioning_found_device_cb cb;

	RETV_IF(NULL == p, G_SOURCE_REMOVE);

	container = p;

	if (container->cb_data->cb) {
		cb = container->cb_data->cb;
		if (IOTCON_FUNC_STOP == cb(container->device, IOTCON_ERROR_NONE,
					container->cb_data->user_data)) {
			INFO("Stop the callback");
			container->cb_data->cb = NULL;

			ret = icl_ioty_mutex_lock();
			if (IOTCON_ERROR_NONE != ret) {
				ERR("icl_ioty_mutex_lock() Fail(%d)", ret);
				return G_SOURCE_REMOVE;
			}

			OCCancel(container->cb_data->handle, OC_LOW_QOS, NULL, 0);
			icl_ioty_mutex_unlock();
		}
	}

	_provisioning_free_find_cb_container(container);

	return G_SOURCE_REMOVE;
}


static OCStackApplicationResult _provisioning_find_security_version_cb(void *ctx,
		OCDoHandle handle, OCClientResponse *resp)
{
	FN_CALL;
	int ret;
	size_t payload_size;
	uint8_t *security_data;
	OicSecVer_t *ver = NULL;
	icl_provisioning_find_cb_container_s *container = ctx;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_DELETE_TRANSACTION);

	_provisioning_find_mutex_lock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);
	if (0 == container->timer_id) {
		_provisioning_find_mutex_unlock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);
		return OC_STACK_DELETE_TRANSACTION;
	}

	g_source_remove(container->timer_id);
	container->timer_id = 0;
	_provisioning_find_mutex_unlock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);

	if (NULL == resp->payload) {
		ret = icl_provisioning_device_create(container->oic_device, &container->device);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icl_provisioning_device_create() Fail(%d)", ret);
			return OC_STACK_DELETE_TRANSACTION;
		}

		icl_provisioning_device_print(container->device);

		g_idle_add(_provisioning_find_idle_cb, container);
		return OC_STACK_DELETE_TRANSACTION;
	}
	RETVM_IF(PAYLOAD_TYPE_SECURITY != resp->payload->type,
			OC_STACK_DELETE_TRANSACTION, "Invalid payload type(%d)", resp->payload->type);
	RETV_IF(OC_STACK_OK != resp->result, OC_STACK_DELETE_TRANSACTION);

	/* cbor payload -> ver */
	security_data = ((OCSecurityPayload*)resp->payload)->securityData;
	payload_size = ((OCSecurityPayload*)resp->payload)->payloadSize;

	ret = CBORPayloadToVer(security_data, payload_size, &ver);
	if (NULL == ver || OC_STACK_OK != ret) {
		ERR("CBORPayloadToVer() Fail(%d)", ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	snprintf(container->oic_device->secVer, sizeof(container->oic_device->secVer),
			"%s", ver->secv);

	ret = icl_provisioning_device_create(container->oic_device, &container->device);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_provisioning_device_create() Fail(%d)", ret);
		return OC_STACK_DELETE_TRANSACTION;
	}

	icl_provisioning_device_print(container->device);

	g_idle_add(_provisioning_find_idle_cb, container);

	return OC_STACK_DELETE_TRANSACTION;
}


static int _provisioning_parse_oic_dev_address(OCDevAddr *dev_addr,
		OCConnectivityType conn_type, char **host_address)
{
	char temp[PATH_MAX] = {0};

	RETV_IF(NULL == dev_addr, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == host_address ,IOTCON_ERROR_INVALID_PARAMETER);

	if (CT_ADAPTER_IP & conn_type) {
		if (CT_IP_USE_V4 & conn_type) {
			snprintf(temp, sizeof(temp), "%s:%d", dev_addr->addr,
					dev_addr->port);
		} else if (CT_IP_USE_V6 & conn_type) {
			snprintf(temp, sizeof(temp), "[%s]:%d", dev_addr->addr,
					dev_addr->port);
		} else {
			ERR("Invalid Connectivity Type(%d)", conn_type);
			return IOTCON_ERROR_INVALID_PARAMETER;
		}
	} else {
		ERR("Invalid Connectivity Type(%d)", conn_type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	*host_address = strdup(temp);

	return IOTCON_ERROR_NONE;
}


static void* _provisioning_find_security_version(void *user_data)
{
	FN_CALL;
	int ret;
	OCCallbackData cbdata = {0};
	char *host_address;
	char uri[PATH_MAX] = {0};
	const char *version_uri = "/oic/sec/ver";
	icl_provisioning_find_cb_container_s *container = user_data;

	ret = _provisioning_parse_oic_dev_address(&container->oic_device->endpoint,
			container->oic_device->connType, &host_address);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_parse_oic_dev_address() Fail(%d)", ret);
		return NULL;
	}

	snprintf(uri, sizeof(uri), "%s%s%s", IC_IOTY_COAP, host_address, version_uri);
	DBG("uri : %s", uri);

	cbdata.cb = _provisioning_find_security_version_cb;
	cbdata.context = container;

	ret = icl_ioty_mutex_lock();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_mutex_lock() Fail(%d)", ret);
		return NULL;
	}
	ret = OCDoResource(&container->handle, OC_REST_DISCOVER, uri, 0, 0,
			container->oic_device->connType, OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_mutex_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCDoResource() Fail(%d)", ret);
		return NULL;
	}

	/* timeout */
	container->timer_id = g_timeout_add_seconds(container->cb_data->timeout,
			_provisioning_find_internal_timeout, container);

	return NULL;
}


static OCStackApplicationResult _provisioning_find_secure_port_cb(void *ctx,
		OCDoHandle handle, OCClientResponse *resp)
{
	FN_CALL;
	int ret;
	OCResourcePayload *payload;
	icl_provisioning_find_cb_container_s *container, *context;

	RETV_IF(NULL == ctx, OC_STACK_DELETE_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_DELETE_TRANSACTION);
	RETV_IF(NULL == resp->payload, OC_STACK_DELETE_TRANSACTION);
	RETV_IF(OC_STACK_OK != resp->result, OC_STACK_DELETE_TRANSACTION);
	RETVM_IF(PAYLOAD_TYPE_DISCOVERY != resp->payload->type,
			OC_STACK_DELETE_TRANSACTION, "Invalid payload type(%d)", resp->payload->type);

	container = ctx;

	_provisioning_find_mutex_lock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);
	if (0 == container->timer_id) {
		_provisioning_find_mutex_unlock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);
		return OC_STACK_DELETE_TRANSACTION;
	}

	g_source_remove(container->timer_id);
	container->timer_id = 0;
	_provisioning_find_mutex_unlock(ICL_PROVISIONING_MUTEX_INTERNAL_FIND, container);

	payload = ((OCDiscoveryPayload*)resp->payload)->resources;
	if (NULL == payload || false == payload->secure) {
		ERR("Not find secure port");
		return OC_STACK_DELETE_TRANSACTION;
	}

	container->oic_device->securePort = payload->port;

	context = calloc(1, sizeof(icl_provisioning_find_cb_container_s));
	if (NULL == context) {
		ERR("calloc() Fail(%d)", errno);
		return OC_STACK_DELETE_TRANSACTION;
	}
	context->oic_device = container->oic_device;
	context->device = container->device;
	context->cb_data = _provisioning_find_cb_ref(container->cb_data);
	container->oic_device = NULL;
	container->device = NULL;

	ret = _provisioning_find_thread(ICL_PROVISIONING_FIND_SECURITY_VERSION,
			context);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_find_thread() Fail(%d)", ret);
		_provisioning_free_find_cb_container(context);
		return OC_STACK_DELETE_TRANSACTION;
	}

	return OC_STACK_DELETE_TRANSACTION;
}


static void* _provisioning_find_secure_port(void *user_data)
{
	FN_CALL;
	int ret;
	char *host_address;
	char uri[PATH_MAX] = {0};
	OCCallbackData cbdata = {0};
	const char *doxm_type = "oic.sec.doxm";
	icl_provisioning_find_cb_container_s *container = user_data;

	ret = _provisioning_parse_oic_dev_address(&container->oic_device->endpoint,
			container->oic_device->connType, &host_address);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_parse_oic_dev_address() Fail(%d)", ret);
		_provisioning_free_find_cb_container(container);
		return NULL;
	}

	snprintf(uri, sizeof(uri), "%s%s%s?%s=%s", IC_IOTY_COAP, host_address,
			OC_RSRVD_WELL_KNOWN_URI, OC_RSRVD_RESOURCE_TYPE, doxm_type);

	cbdata.cb = _provisioning_find_secure_port_cb;
	cbdata.cd = _provisioning_free_find_cb_container;
	cbdata.context = container;

	ret = icl_ioty_mutex_lock();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_mutex_lock() Fail(%d)", ret);
		_provisioning_free_find_cb_container(container);
		return NULL;
	}
	_provisioning_find_mutex_lock(ICL_PROVISIONING_MUTEX_FIND, container->cb_data);
	if (0 == container->cb_data->timer_id) {
		_provisioning_free_find_cb_container(container);
		return NULL;
	}
	_provisioning_find_mutex_unlock(ICL_PROVISIONING_MUTEX_FIND, container->cb_data);

	ret = OCDoResource(&container->handle, OC_REST_DISCOVER, uri, 0, 0,
			container->oic_device->connType, OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_mutex_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCDoResource() Fail(%d)", ret);
		_provisioning_free_find_cb_container(container);
		return NULL;
	}

	/* timeout */
	container->timer_id = g_timeout_add_seconds(container->cb_data->timeout,
			_provisioning_find_internal_timeout, container);

	return NULL;
}


static int _provisioning_find_thread(int type, void *user_data)
{
	int ret;
	pthread_t thread;
	void *thread_routine;

	switch (type) {
	case ICL_PROVISIONING_FIND_SECURE_PORT:
		thread_routine = _provisioning_find_secure_port;
		break;
	case ICL_PROVISIONING_FIND_SECURITY_VERSION:
		thread_routine = _provisioning_find_security_version;
		break;
	default:
		ERR("Invalid Type(%d)", type);
		return IOTCON_ERROR_INVALID_TYPE;
	}

	ret = pthread_create(&thread, NULL, thread_routine, user_data);
	if (0 != ret) {
		ERR("pthread_create() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}
	pthread_detach(thread);

	return IOTCON_ERROR_NONE;
}


static OCStackApplicationResult _provisioning_find_cb(void *ctx, OCDoHandle handle,
		OCClientResponse *resp)
{
	int ret;
	OicUuid_t device_id;
	size_t payload_size;
	uint8_t *security_data;
	OicSecDoxm_t *doxm = NULL;
	icl_provisioning_find_cb_container_s *container;

	RETV_IF(NULL == ctx, OC_STACK_KEEP_TRANSACTION);
	RETV_IF(NULL == resp, OC_STACK_KEEP_TRANSACTION);
	if (NULL == resp->payload)
		return OC_STACK_KEEP_TRANSACTION;
	RETV_IF(OC_STACK_OK != resp->result, OC_STACK_KEEP_TRANSACTION);
	RETVM_IF(PAYLOAD_TYPE_SECURITY != resp->payload->type,
			OC_STACK_KEEP_TRANSACTION, "Invalid payload type(%d)", resp->payload->type);

	/* cbor payload -> doxm */
	security_data = ((OCSecurityPayload*)resp->payload)->securityData;
	payload_size = ((OCSecurityPayload*)resp->payload)->payloadSize;

	ret = CBORPayloadToDoxm(security_data, payload_size, &doxm);
	if (NULL == doxm || OC_STACK_OK != ret) {
		ERR("CBORPayloadToDoxm() Fail(%d)", ret);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* device id */
	ret = GetDoxmDevOwnerId(&device_id);
	if (OC_STACK_OK != ret) {
		ERR("GetDoxmDevOwnerId() Fail(%d)", ret);
		DeleteDoxmBinData(doxm);
		return OC_STACK_KEEP_TRANSACTION;
	}

	/* check whether the device is not mine */
	if (true == doxm->owned &&
			IC_EQUAL != memcmp(&doxm->owner.id, &device_id.id, sizeof(device_id.id))) {
		DBG("discovered device is not owned by me");
		DeleteDoxmBinData(doxm);
		return OC_STACK_KEEP_TRANSACTION;
	}

	container = calloc(1, sizeof(icl_provisioning_find_cb_container_s));
	if (NULL == container) {
		ERR("calloc() Fail(%d)", errno);
		DeleteDoxmBinData(doxm);
		return OC_STACK_KEEP_TRANSACTION;
	}

	container->cb_data = _provisioning_find_cb_ref(ctx);

	ret = _provisioning_find_create_device(resp->devAddr.addr, resp->devAddr.port,
			resp->devAddr.adapter, resp->connType, doxm, &container->oic_device);
	if (OC_STACK_OK != ret) {
		ERR("_provisioning_find_create_device() Fail(%d)", ret);
		DeleteDoxmBinData(doxm);
		_provisioning_free_find_cb_container(container);
		return OC_STACK_KEEP_TRANSACTION;
	}

	ret = _provisioning_find_thread(ICL_PROVISIONING_FIND_SECURE_PORT,
			container);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("_provisioning_find_thread() Fail(%d)", ret);
		_provisioning_free_find_cb_container(container);
		return OC_STACK_KEEP_TRANSACTION;
	}

	return OC_STACK_KEEP_TRANSACTION;
}


API int iotcon_provisioning_find_device(iotcon_provisioning_find_e type,
		iotcon_provisioning_found_device_cb cb, void *user_data)
{
	FN_CALL;
	int ret;
	const char *query;
	OCCallbackData cbdata = {0};
	icl_provisioning_find_cb_s *cb_data;

	RETV_IF(false == ic_utils_check_ocf_feature(), IOTCON_ERROR_NOT_SUPPORTED);
	RETV_IF(NULL == cb, IOTCON_ERROR_INVALID_PARAMETER);

	switch (type) {
	case IOTCON_PROVISIONING_FIND_OWNED:
		query = ICL_PROVISIONING_OWNED_MULTICAST_QUERY;
		break;
	case IOTCON_PROVISIONING_FIND_UNOWNED:
		query = ICL_PROVISIONING_UNOWNED_MULTICAST_QUERY;
		break;
	case IOTCON_PROVISIONING_FIND_ALL:
		query = ICL_PROVISIONING_ALL_MULTICAST_QUERY;
		break;
	default:
		ERR("Invalid Type(%d)", type);
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	cb_data = calloc(1, sizeof(icl_provisioning_find_cb_s));
	if (NULL == cb_data) {
		ERR("calloc() Fail(%d)", errno);
		return IOTCON_ERROR_OUT_OF_MEMORY;
	}
	cb_data->cb = cb;
	cb_data->user_data = user_data;
	cb_data->ref_count = 1;

	cbdata.cb = _provisioning_find_cb;
	cbdata.cd = _provisioning_free_cb_data;
	cbdata.context = cb_data;

	ret = icl_ioty_mutex_lock();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icl_ioty_mutex_lock() Fail(%d)", ret);
		_provisioning_free_cb_data(cb_data);
		return ret;
	}
	ret = OCDoResource(&cb_data->handle, OC_REST_DISCOVER, query, 0, 0, CT_DEFAULT,
			OC_LOW_QOS, &cbdata, NULL, 0);
	icl_ioty_mutex_unlock();
	if (OC_STACK_OK != ret) {
		ERR("OCDoResource() Fail(%d)", ret);
		_provisioning_free_cb_data(cb_data);
		return ic_ioty_parse_oic_error(ret);
	}

	/* timeout */
	iotcon_get_timeout(&cb_data->timeout);
	cb_data->timer_id = g_timeout_add_seconds(cb_data->timeout,
			_provisioning_find_timeout, cb_data);

	return IOTCON_ERROR_NONE;
}
