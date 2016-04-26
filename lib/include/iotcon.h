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
#ifndef __IOT_CONNECTIVITY_H__
#define __IOT_CONNECTIVITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <iotcon-types.h>
#include <iotcon-errors.h>
#include <iotcon-server.h>
#include <iotcon-client.h>

/**
 * @file iotcon.h
 */

/**
 *
 * @addtogroup CAPI_IOT_CONNECTIVITY_MODULE
 *
 * @section CAPI_IOT_CONNECTIVITY_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 * - http://tizen.org/feature/iot.oic\n
 * - http://tizen.org/feature/iot.oic.security\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * If the feature(http://tizen.org/feature/iot.oic.security) for OIC security architecture
 * is applied, you should set a file using iotcon_set_persistent_storage.
 * Then applications communicate with each other based on OIC security architecture.
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="https://developer.tizen.org/development/tools/native-tools/manifest-text-editor#feature"><b>Feature Element</b>.</a>
 *
 * @{
 */

/**
 * @brief Sets persistent storage.
 * @details This API set @a path to point to storage for handle secure virtual resources.
 *
 * @since_tizen 3.0
 *
 * @remarks If the device has the feature("http://tizen.org/feature/iot.oic.security"),
 * you must call this function before iotcon_initialize() is called.\n
 * A file that is CBOR(Concise Binary Object Representation)-format must already exist
 * in @a path. We recommend to use application-local file for @a path.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_IOTIVITY  Iotivity errors
 *
 * @post iotcon_initialize()
 */
int iotcon_set_persistent_storage(const char *path);

/**
 * @brief Connects to the iotcon service.
 * @details Call this function to start IoTCon.
 *
 * @since_tizen 3.0
 * @privlevel public
 * @privilege %http://tizen.org/privilege/network.get
 * @privilege %http://tizen.org/privilege/internet
 *
 * @remarks You must free all resources of the IoTCon by calling iotcon_deinitialize()
 * if IoTCon API is no longer needed.
 *
 * @return  0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_PERMISSION_DENIED Permission denied
 *
 * @pre iotcon_set_persistent_storage() should be called
 * when the security feature("http://tizen.org/feature/iot.oic.security") is applied.
 *
 * @see iotcon_deinitialize()
 */
int iotcon_initialize(void);

/**
 * @brief Disconnects from the iotcon service.
 * @details Frees the resources allocated to IoTCon.
 *
 * @since_tizen 3.0
 *
 * @remarks This function must be called if IoTCon API is no longer needed.
 *
 * @return void
 *
 * @see iotcon_initialize()
 */
void iotcon_deinitialize(void);

/**
 * @brief Gets the timeout seconds of asynchronous API.
 * @details This API get the timeout of iotcon_get_device_info(),
 * iotcon_get_platform_info(), iotcon_find_resource(),
 * iotcon_remote_resource_get(), iotcon_remote_resource_put(),
 * iotcon_remote_resource_post() and iotcon_remote_resource_delete().
 *
 * @since_tizen 3.0
 *
 * @param[out] timeout_seconds Seconds for timeout
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @pre iotcon_initialize() should be called to initialize.
 *
 * @see iotcon_set_timeout()
 */
int iotcon_get_timeout(int *timeout_seconds);

/**
 * @brief Sets the timeout seconds of asynchronous APIs.
 * @details This API set the timeout of iotcon_get_device_info(),
 * iotcon_get_platform_info(), iotcon_find_resource(),
 * iotcon_remote_resource_get(), iotcon_remote_resource_put(),
 * iotcon_remote_resource_post() and iotcon_remote_resource_delete().\n
 * Default timeout interval value is 30.
 *
 * @since_tizen 3.0
 *
 * @param[in] timeout_seconds Seconds for timeout (must be in range from 1 to 3600)
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @pre iotcon_initialize() should be called to initialize.
 *
 * @see iotcon_get_timeout()
 */
int iotcon_set_timeout(int timeout_seconds);

/**
 * @brief Gets the polling interval(milliseconds) of iotcon.
 * @details This API gets the polling interval of iotcon.
 * Default polling interval is 100 milliseconds.
 *
 * @since_tizen 3.0
 *
 * @param[out] interval Milliseconds for polling interval
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @pre iotcon_initialize() should be called to initialize.
 *
 * @see iotcon_set_timeout()
 */
int iotcon_polling_get_interval(int *interval);

/**
 * @brief Sets the polling interval(milliseconds) of iotcon.
 * @details This API sets the polling interval of iotcon.
 * The closer to 0, the faster it operates. It is invoked immediately for changing the interval.
 * Default polling interval is 100 milliseconds. If you want the faster operation,
 * we recommend you set 10 milliseconds for polling interval.
 *
 * @since_tizen 3.0
 *
 * @param[in] interval Milliseconds for polling interval (must be in range from 1 to 999)
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_INVALID_PARAMETER Invalid parameter
 *
 * @pre iotcon_initialize() should be called to initialize.
 *
 * @see iotcon_polling_get_interval()
 */
int iotcon_polling_set_interval(int interval);

/**
 * @brief Invokes a next message from a queue for receiving messages from others, immediately.
 * @details This API invokes a next message from a queue for receiving messages from others, immediately.\n
 * After calling the API, it continues the polling with existing interval.
 *
 * @since_tizen 3.0
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 *
 * @pre iotcon_initialize() should be called to initialize.
 */
int iotcon_polling_invoke(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __IOT_CONNECTIVITY_H__ */
