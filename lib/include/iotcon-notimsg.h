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
#ifndef __IOT_CONNECTIVITY_MANAGER_SERVER_NOTIMSG_H__
#define __IOT_CONNECTIVITY_MANAGER_SERVER_NOTIMSG_H__

#include <iotcon-constant.h>

/**
 * @file iotcon-notimsg.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_NOTIMSG_MODULE Notimsg
 *
 * @brief Iotcon Notimsg provides API to create a message to notify to client.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_NOTIMSG_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @{
 */

/**
 * @brief Creates a new notifications message handle.
 * @details @a iface could be one of #iotcon_interface_e.
 *
 * @since_tizen 3.0
 *
 * @param[in] repr The handle of the representation
 * @param[in] iface The resource interface
 * @param[out] notimsg_handle The generated notifications message handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_remote_resource_observe_cb()
 * @see iotcon_remote_resource_observer_start()
 * @see iotcon_remote_resource_observer_stop()
 * @see iotcon_resource_notify()
 */
int iotcon_notimsg_create(iotcon_representation_h repr, iotcon_interface_e iface,
		iotcon_notimsg_h *notimsg_handle);

/**
 * @brief Releases a notifications message handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] msg The handle of the notifications message
 *
 * @return void
 *
 * @see iotcon_notimsg_create()
 */
void iotcon_notimsg_destroy(iotcon_notimsg_h msg);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_SERVER_NOTIMSG_H__ */
