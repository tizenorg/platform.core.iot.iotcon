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
#include <glib.h>
#include <gio/gio.h>
#include <cynara-client.h>
#include <cynara-session.h>
#include <cynara-creds-gdbus.h>

#include "iotcon.h"
#include "ic-common.h"
#include "icd.h"
#include "icd-cynara.h"

static cynara *_cynara;

int icd_cynara_init()
{
	int ret;
	ret = cynara_initialize(&_cynara, NULL);

	if (CYNARA_API_SUCCESS != ret) {
		ERR("cynara_initialize() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}

	return IOTCON_ERROR_NONE;
}

void icd_cynara_deinit()
{
	if (_cynara)
		cynara_finish(_cynara);

	_cynara = NULL;
}


int icd_cynara_check(GDBusMethodInvocation *invocation, const char **privileges)
{
	FN_CALL;
	int i = 0;
	int ret;
	pid_t pid;
	GDBusConnection *conn = NULL;
	char *client = NULL;
	char *user = NULL;
	char *sender = NULL;
	char *session = NULL;

	RETV_IF(NULL == _cynara, IOTCON_ERROR_SYSTEM);
	RETV_IF(NULL == invocation, IOTCON_ERROR_INVALID_PARAMETER);
	RETV_IF(NULL == privileges, IOTCON_ERROR_INVALID_PARAMETER);

	conn = g_dbus_method_invocation_get_connection(invocation);
	if (NULL == conn) {
		ERR("g_dbus_method_invocation_get_connection() return NULL");
		return IOTCON_ERROR_SYSTEM;
	}

	sender = (char *)g_dbus_method_invocation_get_sender(invocation);
	if (NULL == sender) {
		ERR("g_dbus_method_invocation_get_sender() return NULL");
		return IOTCON_ERROR_SYSTEM;
	}

	ret = cynara_creds_gdbus_get_client(conn, sender, CLIENT_METHOD_SMACK, &client);
	if (CYNARA_API_SUCCESS != ret) {
		ERR("cynara_creds_dbus_get_client() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = cynara_creds_gdbus_get_user(conn, sender, USER_METHOD_UID, &user);
	if (CYNARA_API_SUCCESS != ret) {
		ERR("cynara_creds_dbus_get_user() Fail(%d)", ret);
		free(client);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = cynara_creds_gdbus_get_pid(conn, sender, &pid);
	if (CYNARA_API_SUCCESS != ret) {
		ERR("cynara_creds_gdbus_get_pid() Fail(%d)", ret);
		free(user);
		free(client);
		return IOTCON_ERROR_SYSTEM;
	}

	session = cynara_session_from_pid(pid);
	if (NULL == session) {
		ERR("cynara_session_from_pid() return NULL");
		free(user);
		free(client);
		return IOTCON_ERROR_SYSTEM;
	}

	while (privileges[i]) {
		SECURE_DBG("privileges[%d]: %s, user: %s, client: %s", i, privileges[i], user, client);
		ret = cynara_check(_cynara, client, session, user, privileges[i]);
		if (CYNARA_API_ACCESS_DENIED == ret) {
			ERR("Denied (%s)", privileges[i]);
			free(session);
			free(user);
			free(client);
			return IOTCON_ERROR_PERMISSION_DENIED;
		} else if (CYNARA_API_ACCESS_ALLOWED != ret) {
			ERR("cynara_check(%s) Fail(%d)", privileges[i], ret);
			free(session);
			free(user);
			free(client);
			return IOTCON_ERROR_SYSTEM;
		}
		i++;
	}
	free(session);
	free(user);
	free(client);
	return IOTCON_ERROR_NONE;
}

