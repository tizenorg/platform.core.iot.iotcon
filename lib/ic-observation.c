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
#include <errno.h>
#include <glib.h>

#include "iotcon-struct.h"
#include "ic-common.h"

API void iotcon_observers_free(iotcon_observers_h observers)
{
	RET_IF(NULL == observers);

	g_list_free(observers);
}


/* If you want to make a new list, then you should set observers is NULL. */
API iotcon_observers_h iotcon_observers_append(iotcon_observers_h observers,
		int obs_id)
{
	return g_list_append(observers, GUINT_TO_POINTER(obs_id));
}


API iotcon_observers_h iotcon_observers_remove(iotcon_observers_h observers,
		int obs_id)
{
	RETV_IF(NULL == observers, observers);

	return g_list_remove(observers, GUINT_TO_POINTER(obs_id));
}

