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
#ifndef __IOT_CONNECTIVITY_MANAGER_TEST_LOG_H__
#define __IOT_CONNECTIVITY_MANAGER_TEST_LOG_H__

#define ICTEST_LOGRED		"\033[0;31m"
#define ICTEST_LOGGREEN		"\033[0;32m"
#define ICTEST_LOGBROWN		"\033[0;33m"
#define ICTEST_LOGBLUE		"\033[0;34m"
#define ICTEST_LOGEND		"\033[0;m"

#undef _DBG
#undef _INFO
#undef _WARN
#undef _ERR

#undef DBG
#undef INFO
#undef WARN
#undef ERR

#define TIZEN_DEBUG_ENABLE
#define LOG_TAG "ICTEST"
#include <dlog.h>

#define _DBG(fmt, arg...) SLOGD(fmt, ##arg)
#define _INFO(fmt, arg...) SLOGI(fmt, ##arg)
#define _WARN(fmt, arg...) SLOGW(fmt, ##arg)
#define _ERR(fmt, arg...) SLOGE(fmt, ##arg)

#if 0
#define _DBG(fmt, arg...) \
	printf("[IoTConTest]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define _INFO(fmt, arg...) \
	printf("[IoTConTest]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define _WARN(fmt, arg...) \
	printf("[IoTConTest]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define _ERR(fmt, arg...) \
	printf("[IoTConTest]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#endif

#define ICTEST_DEBUGGING

#ifdef ICTEST_DEBUGGING

#define FN_CALL _INFO(">>>>>>>> called")
#define FN_END _INFO("<<<<<<<< ended")
#define DBG(fmt, arg...) _DBG(fmt, ##arg)
#define WARN(fmt, arg...) _WARN(ICTEST_LOGBROWN fmt ICTEST_LOGEND, ##arg)
#define ERR(fmt, arg...) _ERR(ICTEST_LOGRED fmt ICTEST_LOGEND, ##arg)
#define INFO(fmt, arg...) _INFO(ICTEST_LOGBLUE fmt ICTEST_LOGEND, ##arg)
#define SECURE_DBG(fmt, arg...) SECURE_SLOGD(fmt, ##arg)
#define SECURE_ERR(fmt, arg...) SECURE_SLOGE(fmt, ##arg)
#define ABORT() abort()

#else /* ICTEST_DEBUGGING */

#define FN_CALL
#define FN_END
#define DBG(fmt, arg...)
#define WARN(fmt, arg...)
#define ERR(fmt, arg...) _ERR(fmt, ##arg)
#define INFO(fmt, arg...)
#define SECURE_DBG(fmt, arg...)
#define SECURE_ERR(fmt, arg...) SECURE_SLOGE(fmt, ##arg)
#define ABORT()

#endif /* ICTEST_DEBUGGING */

#define RET_IF(expr) \
	do { \
		if (expr) { \
			ERR("(%s)", #expr); \
			return; \
		}\
	} while(0)

#define RETV_IF(expr, val) \
	do {\
		if (expr) { \
			ERR("(%s)", #expr); \
			return (val); \
		} \
	} while(0)

#define RETM_IF(expr, fmt, arg...) \
	do {\
		if (expr) { \
			ERR(fmt, ##arg); \
			return; \
		}\
	} while(0)

#define RETVM_IF(expr, val, fmt, arg...) \
	do {\
		if (expr) { \
			ERR(fmt, ##arg); \
			return (val); \
		} \
	} while(0)

#define ERR_IF(expr) \
	do { \
		if (expr) { \
			ERR("(%s)", #expr); \
		} \
	} while (0)

#define WARN_IF(expr) \
	do { \
		if (expr) { \
			WARN("(%s)", #expr); \
		} \
	} while (0)

#endif //__IOT_CONNECTIVITY_MANAGER_TEST_LOG_H__
