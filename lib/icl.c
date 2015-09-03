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
#include <glib-object.h>

#include "icl.h"
#include "icl-dbus.h"

API int iotcon_open(void)
{
	int ret;

#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init();
#endif

	ret = icl_dbus_start();
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_start() Fail(%d)", ret);

	return ret;
}


API int iotcon_close(void)
{
	int ret;

	ret = icl_dbus_stop();
	if (IOTCON_ERROR_NONE != ret)
		ERR("icl_dbus_stop() Fail(%d)", ret);

	return ret;
}
