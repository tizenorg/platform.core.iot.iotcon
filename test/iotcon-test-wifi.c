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
#include <wifi.h>

#include "test.h"

static void _check_wifi_state()
{
	int ret;
	wifi_ap_h ap;
	wifi_connection_state_e connection_state;
	bool activated;
	char *essid;

	ret = wifi_initialize();
	if (WIFI_ERROR_NONE != ret) {
		ERR("wifi_initialize() Fail(%d)", ret);
		printf("wifi_initialize() Fail(%d)\n", ret);
		return;
	}

	ret = wifi_is_activated(&activated);
	if (WIFI_ERROR_NONE != ret) {
		ERR("wifi_is_activated() Fail(%d)", ret);
		printf("wifi_is_activated() Fail(%d)\n", ret);
		wifi_deinitialize();
		return;
	} else if (false == activated) {
		ERR("wifi is not activated");
		printf("wifi is not activate\n");
		wifi_deinitialize();
		return;
	}

	ret = wifi_get_connection_state(&connection_state);
	if (WIFI_ERROR_NONE != ret) {
		ERR("wifi_get_connection_state() Fail(%d)", ret);
		printf("wifi_get_connection_state() Fail(%d)\n", ret);
		wifi_deinitialize();
		return;
	}

	switch (connection_state) {
	case WIFI_CONNECTION_STATE_CONNECTED:
		INFO("WIFI_CONNECTION_STATE_CONNECTED");
		printf("WIFI_CONNECTION_STATE_CONNECTED\n");
		ret = wifi_get_connected_ap(&ap);
		if (WIFI_ERROR_NONE != ret) {
			ERR("wifi_get_connected_ap() Fail(%d)", ret);
			printf("wifi_get_connected_ap() Fail(%d)\n", ret);
			break;
		}
		ret = wifi_ap_get_essid(ap, &essid);
		if (WIFI_ERROR_NONE != ret) {
			ERR("wifi_ap_get_essid() Fail(%d)", ret);
			printf("wifi_ap_get_essid() Fail(%d)\n", ret);
			wifi_ap_destroy(ap);
			break;
		}
		INFO("ssid: %s", essid);
		printf("ssid: %s\n", essid);
		free(essid);
		wifi_ap_destroy(ap);
		break;
	case WIFI_CONNECTION_STATE_FAILURE:
		ERR("WIFI_CONNECTION_STATE_FAILURE");
		printf("WIFI_CONNECTION_STATE_FAILURE\n");
		break;
	case WIFI_CONNECTION_STATE_DISCONNECTED:
		ERR("WIFI_CONNECTION_STATE_DISCONNECTED");
		printf("WIFI_CONNECTION_STATE_DISCONNECTED\n");
		break;
	case WIFI_CONNECTION_STATE_ASSOCIATION:
		ERR("WIFI_CONNECTION_STATE_ASSOCIATION");
		printf("WIFI_CONNECTION_STATE_ASSOCIATION\n");
		break;
	case WIFI_CONNECTION_STATE_CONFIGURATION:
		ERR("WIFI_CONNECTION_STATE_CONFIGURATION");
		printf("WIFI_CONNECTION_STATE_CONFIGURATION\n");
		break;
	default:
		ERR("Invalid connection state(%d)", connection_state);
		printf("Invalid connection state(%d)", connection_state);
	}

	wifi_deinitialize();
}

int main(int argc, char **argv)
{
	_check_wifi_state();
	return 0;
}
