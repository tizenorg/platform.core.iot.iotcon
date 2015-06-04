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

#include "icd.h"
#include "icd-dbus.h"


int main(int argc, char **argv)
{
	guint id;
	GMainLoop *loop;

#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif

	loop = g_main_loop_new(NULL, FALSE);

	id = icd_dbus_init();

	g_main_loop_run(loop);

	icd_dbus_deinit(id);
	g_main_loop_unref(loop);

	return 0;
}
