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
#ifndef __IOT_CONNECTIVITY_INTERNAL_COMMON_UTILITY_H__
#define __IOT_CONNECTIVITY_INTERNAL_COMMON_UTILITY_H__

#include <time.h>
#include <octypes.h>
#include "iotcon-types.h"

#define IC_EQUAL 0

char* ic_utils_strdup(const char *src);
bool ic_utils_check_oic_feature_supported();
int ic_utils_get_platform_info(OCPlatformInfo *platform_info);
void ic_utils_free_platform_info(OCPlatformInfo *platform_info);
bool ic_utils_check_permission();

void ic_utils_mutex_lock(int type);
void ic_utils_mutex_unlock(int type);

void ic_utils_cond_signal(int type);
void ic_utils_cond_timedwait(int cond_type, int mutex_type, int polling_interval);

enum IC_UTILS_MUTEX {
	IC_UTILS_MUTEX_INIT,
	IC_UTILS_MUTEX_IOTY,
	IC_UTILS_MUTEX_POLLING,
	IC_UTILS_MUTEX_MAX
};

enum IC_UTILS_COND {
	IC_UTILS_COND_POLLING,
	IC_UTILS_COND_MAX
};

#endif /* __IOT_CONNECTIVITY_INTERNAL_COMMON_UTILITY_H__ */
