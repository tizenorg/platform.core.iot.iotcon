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
#ifndef __IOT_CONNECTIVITY_MANAGER_INTERNAL_DEVICE_H__
#define __IOT_CONNECTIVITY_MANAGER_INTERNAL_DEVICE_H__

struct ic_device_info {
	char *device_name;
	char *host_name;
	char *device_uuid;
	char *content_type;
	char *version;
	char *manufacturer_name;
	char *manufacturer_url;
	char *model_number;
	char *date_of_manufacture;
	char *platform_ver;
	char *firmware_ver;
	char *support_url;
};

#endif /* __IOT_CONNECTIVITY_MANAGER_INTERNAL_DEVICE_H__ */
