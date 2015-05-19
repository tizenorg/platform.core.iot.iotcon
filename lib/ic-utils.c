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
#include <string.h>
#include <errno.h>

#include "iotcon-struct.h"
#include "ic-common.h"
#include "ic-utils.h"

char* ic_utils_strdup(const char *src)
{
	char *dest = NULL;

	RETV_IF(NULL == src, NULL);

	errno = 0;
	dest = strdup(src);
	if (NULL == dest) {
		ERR("strdup() Fail(%d)", errno);
		return NULL;
	}

	return dest;
}


static iotcon_str_list_s* _ic_str_list_last(iotcon_str_list_s *str_list)
{
	RETV_IF(NULL == str_list, NULL);

	while (str_list->next)
		str_list = str_list->next;

	return str_list;
}


API void iotcon_str_list_free(iotcon_str_list_s *str_list)
{
	iotcon_str_list_s *cur_node = NULL;

	RET_IF(NULL == str_list);

	while (cur_node) {
		cur_node = str_list;
		str_list = str_list->next;
		free(cur_node->string);
		free(cur_node);
	}
}


/* If you want a new list, then you should set str_list is NULL. */
API iotcon_str_list_s* iotcon_str_list_append(iotcon_str_list_s *str_list,
		const char *string)
{
	iotcon_str_list_s *last_node = NULL;
	iotcon_str_list_s *new_node = NULL;

	RETV_IF(NULL == string, str_list);

	new_node = calloc(1, sizeof(iotcon_str_list_s));
	if (NULL == new_node) {
		ERR("calloc() Fail(%d)", errno);
		return NULL;
	}

	new_node->string = ic_utils_strdup(string);

	if (str_list) {
		last_node = _ic_str_list_last(str_list);
		last_node->next = new_node;
		return str_list;
	}
	else {
		return new_node;
	}
}


API iotcon_str_list_s* iotcon_str_list_remove(iotcon_str_list_s *str_list,
		const char *string)
{
	iotcon_str_list_s *cur_node = NULL;
	iotcon_str_list_s *prev_node = NULL;

	RETV_IF(NULL == str_list, NULL);
	RETV_IF(NULL == string, str_list);

	cur_node = str_list;

	while (cur_node) {
		if (STR_EQUAL == strcmp(string, cur_node->string)) {
			if (prev_node)
				prev_node->next = cur_node->next;
			else
				str_list = cur_node->next;

			free(cur_node->string);
			free(cur_node);
			break;
		}
		prev_node = cur_node;
		cur_node = cur_node->next;
	}

	return str_list;
}


API iotcon_str_list_s* iotcon_str_list_clone(iotcon_str_list_s *str_list)
{
	iotcon_str_list_s *new_list = NULL;

	RETV_IF(NULL == str_list, NULL);

	while (str_list) {
		new_list = iotcon_str_list_append(new_list, str_list->string);
		str_list = str_list->next;
	}

	return new_list;
}


API unsigned int iotcon_str_list_length(iotcon_str_list_s *str_list)
{
	unsigned int length = 0;

	RETV_IF(NULL == str_list, 0);

	while (str_list) {
		length++;
		str_list = str_list->next;
	}

	return length;
}


API void iotcon_str_list_foreach(iotcon_str_list_s *str_list,
		iotcon_string_foreach_cb cb, void *user_data)
{
	RET_IF(NULL == str_list);

	while (str_list) {
		cb(str_list->string, user_data);
		str_list = str_list->next;
	}
}


API const char* iotcon_str_list_nth_data(iotcon_str_list_s *str_list, unsigned int n)
{
	int i;

	RETV_IF(NULL == str_list, NULL);

	for (i = 1; i < n; i++) {
		str_list = str_list->next;
		if (NULL == str_list)
			return NULL;
	}

	return str_list->string;
}
