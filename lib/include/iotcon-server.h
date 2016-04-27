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
#ifndef __IOT_CONNECTIVITY_SERVER_H__
#define __IOT_CONNECTIVITY_SERVER_H__

#include <iotcon-resource.h>
#include <iotcon-lite-resource.h>
#include <iotcon-response.h>
#include <iotcon-observers.h>
#include <iotcon-request.h>

/**
 * @file iotcon-server.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE Server
 *
 * @brief Iotcon Server provides API for server side.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_MODULE_HEADER Required Header
 *  \#include <iotcon.h>
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_MODULE_OVERVIEW Overview
 * This API set consists of server side API for
 * @ref CAPI_IOT_CONNECTIVITY_SERVER_RESOURCE_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_SERVER_LITE_RESOURCE_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_SERVER_OBSERVERS_MODULE,
 * @ref CAPI_IOT_CONNECTIVITY_SERVER_REQUEST_MODULE.
 *
 * @{
 */

/**
 * @brief Starts presence of a server.
 * @details Use this function to send server's announcements to clients.\n
 * Server can call this function when online for the first time or come back from offline to online.
 *
 * @since_tizen 3.0
 *
 * @remarks If @a time_to_live is 0, server will set default value as 60 seconds.\n
 * If @a time_to_live is very big, server will set maximum value as (60 * 60 * 24) seconds
 * (24 hours).\n
 * %http://tizen.org/privilege/internet privilege is needed for networking.
 *
 * @param[in] time_to_live The interval of announcing presence in seconds.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_SYSTEM System error
 *
 * @see iotcon_stop_presence()
 * @see iotcon_add_presence_cb()
 * @see iotcon_remove_presence_cb()
 */
int iotcon_start_presence(unsigned int time_to_live);

/**
 * @brief Stops presence of a server.
 * @details Use this function to stop sending server's announcements to clients.
 * Server can call this function when terminating, entering to offline or out of network.
 *
 * @since_tizen 3.0
 *
 * @remarks %http://tizen.org/privilege/internet privilege is needed for networking.
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_NOT_SUPPORTED  Not supported
 * @retval #IOTCON_ERROR_SYSTEM System error
 *
 * @see iotcon_start_presence()
 * @see iotcon_add_presence_cb()
 * @see iotcon_remove_presence_cb()
 */
int iotcon_stop_presence(void);


/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_SERVER_H__ */
