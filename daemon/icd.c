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
#include <gio/gio.h>
#include <glib.h>

#include "icd.h"
#include "icd-dbus.h"
#include "icd-ioty.h"
#include "icd-cynara.h"
#include "icd-ioty-ocprocess.h"

#define ICD_ALL_INTERFACES "0.0.0.0"
#define ICD_RANDOM_PORT 0
#define ICD_KILL_TIMEOUT_DURATION 10

static GThread *icd_thread;
static GMainLoop *icd_loop;
static int icd_kill_timeout;
static int icd_connection;

void icd_unset_kill_timeout()
{
	FN_CALL;
	if (0 == icd_kill_timeout)
		return;

	g_source_remove(icd_kill_timeout);
	icd_kill_timeout = 0;
}

static gboolean _icd_kill_cb(gpointer p)
{
	FN_CALL;
	g_main_loop_quit(icd_loop);
	return G_SOURCE_REMOVE;
}

void icd_set_kill_timeout()
{
	FN_CALL;
	if (icd_kill_timeout)
		return;

	icd_kill_timeout = g_timeout_add_seconds(ICD_KILL_TIMEOUT_DURATION, _icd_kill_cb, NULL);
}


int icd_initialize()
{
	int ret;

	if (true == icd_connection)
		return IOTCON_ERROR_NONE;

	ret = icd_ioty_init(ICD_ALL_INTERFACES, ICD_RANDOM_PORT, &icd_thread);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_init() Fail(%d)", ret);
		return ret;
	}

	ret = icd_ioty_set_device_info();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_set_device_info() Fail(%d)", ret);
		icd_ioty_deinit(icd_thread);
		return ret;
	}

	ret = icd_ioty_set_platform_info();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_set_platform_info() Fail(%d)", ret);
		icd_ioty_deinit(icd_thread);
		return ret;
	}

	icd_connection = true;

	return IOTCON_ERROR_NONE;
}


int main(int argc, char **argv)
{
	int ret;
	guint id;

#if !GLIB_CHECK_VERSION(2, 35, 0)
	g_type_init();
#endif

	icd_loop = g_main_loop_new(NULL, FALSE);

	id = icd_dbus_init();

	ret = icd_cynara_init();
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_cynara_init() Fail(%d)", ret);
		icd_dbus_deinit(id);
		return ret;
	}

	icd_set_kill_timeout();
	g_main_loop_run(icd_loop);

	if (true == icd_connection) {
		icd_ioty_deinit(icd_thread);
		icd_connection = false;
	}

	icd_ioty_unset_device_info_changed_cb();
	icd_cynara_deinit();
	icd_dbus_deinit(id);
	g_main_loop_unref(icd_loop);

	return 0;
}
