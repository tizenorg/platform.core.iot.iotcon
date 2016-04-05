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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_VALUE_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_VALUE_H__

#include <glib.h>

#include "iotcon-types.h"

struct icl_value_s {
	int type;
};

typedef struct {
	int type;
	union {
		int i;
		bool b;
		double d;
		char *s;
	} val;
} icl_basic_s;

typedef struct {
	int type;
	unsigned char *s;
	int len;
} icl_val_byte_str_s;

typedef struct {
	int type;
	struct icl_list_s *list;
} icl_val_list_s;

typedef struct {
	int type;
	struct icl_state_s *state;
} icl_val_state_s;

/**
 * @ingroup CAPI_IOT_CONNECTIVITY_REPRESENTATION_MODULE
 * @brief The handle of representation value.
 * @details iotcon_value_h is an opaque data structure to have variant datatype and
 * store values along with information about the type of that value.\n
 * The range of possible values is determined by the type.\n
 * The type of iotcon_value_h should be one of them.\n
 * #IOTCON_TYPE_INT\n
 * #IOTCON_TYPE_BOOL\n
 * #IOTCON_TYPE_DOUBLE\n
 * #IOTCON_TYPE_STR\n
 * #IOTCON_TYPE_NULL\n
 * #IOTCON_TYPE_BYTE_STR\n
 * #IOTCON_TYPE_LIST\n
 * #IOTCON_TYPE_STATE
 *
 * @since_tizen 3.0
 */
typedef struct icl_value_s* iotcon_value_h;

iotcon_value_h icl_value_create_null();
iotcon_value_h icl_value_create_int(int val);
iotcon_value_h icl_value_create_bool(bool val);
iotcon_value_h icl_value_create_double(double val);
iotcon_value_h icl_value_create_str(const char *val);
iotcon_value_h icl_value_create_byte_str(const unsigned char *val, int len);
iotcon_value_h icl_value_create_list(iotcon_list_h val);
iotcon_value_h icl_value_create_state(iotcon_state_h val);


int icl_value_get_int(iotcon_value_h value, int *val);
int icl_value_get_bool(iotcon_value_h value, bool *val);
int icl_value_get_double(iotcon_value_h value, double *val);
int icl_value_get_str(iotcon_value_h value, char **val);
int icl_value_get_byte_str(iotcon_value_h value, unsigned char **val, int *len);
int icl_value_get_list(iotcon_value_h value, iotcon_list_h *list);
int icl_value_get_state(iotcon_value_h value, iotcon_state_h *state);

void icl_value_destroy(gpointer data);

iotcon_value_h icl_value_clone(iotcon_value_h src);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_VALUE_H__ */
