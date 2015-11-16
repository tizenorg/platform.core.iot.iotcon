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
#ifndef __IOT_CONNECTIVITY_MANAGER_SERVER_OBSERVERS_H__
#define __IOT_CONNECTIVITY_MANAGER_SERVER_OBSERVERS_H__

#include <iotcon-types.h>

/**
 * @file iotcon-observers.h
 */

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_SERVER_MODULE
 * @defgroup CAPI_IOT_CONNECTIVITY_SERVER_OBSERVERS_MODULE Observers
 *
 * @brief Iotcon Observers provides API to manage client observing a resource.
 *
 * @section CAPI_IOT_CONNECTIVITY_SERVER_OBSERVERS_MODULE_HEADER Header
 *  \#include <iotcon.h>
 *
 * @{
 */

/**
 * @brief Creates a new observers handle.
 *
 * @since_tizen 3.0
 *
 * @remarks You must destroy @a observers by calling iotcon_observers_destroy()
 * if @a observers is no longer needed.
 *
 * @param[out] observers A newly allocated list of observers handle
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 * @retval #IOTCON_ERROR_OUT_OF_MEMORY  Out of memory
 *
 * @see iotcon_observers_destroy()
 * @see iotcon_observers_add()
 * @see iotcon_observers_remove()
 */
int iotcon_observers_create(iotcon_observers_h *observers);

/**
 * @brief Destroys a observers handle.
 *
 * @since_tizen 3.0
 *
 * @param[in] observers The handle of the observers
 *
 * @return void
 *
 * @see iotcon_observers_create()
 * @see iotcon_observers_add()
 * @see iotcon_observers_remove()
 */
void iotcon_observers_destroy(iotcon_observers_h observers);

/**
 * @brief Sets a observer id into the observers handle
 *
 * @since_tizen 3.0
 *
 * @param[in] observers The handle of the observers
 * @param[in] obs_id The id to be appended to observers
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_observers_create()
 * @see iotcon_observers_destroy()
 * @see iotcon_observers_remove()
 */
int iotcon_observers_add(iotcon_observers_h observers, int obs_id);

/**
 * @brief Removes id from the observers.
 *
 * @since_tizen 3.0
 *
 * @param[in] observers observers The handle of the observers
 * @param[in] obs_id The id to be removed from observers
 *
 * @return 0 on success, otherwise a negative error value.
 * @retval #IOTCON_ERROR_NONE  Successful
 * @retval #IOTCON_ERROR_INVALID_PARAMETER  Invalid parameter
 *
 * @see iotcon_observers_create()
 * @see iotcon_observers_destroy()
 * @see iotcon_observers_add()
 */
int iotcon_observers_remove(iotcon_observers_h observers, int obs_id);

/**
 * @}
 */

#endif /* __IOT_CONNECTIVITY_MANAGER_SERVER_OBSERVERS_H__ */
