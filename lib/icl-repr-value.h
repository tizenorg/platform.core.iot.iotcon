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
#ifndef __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_VALUE_H__
#define __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_VALUE_H__

#include <stdbool.h>

#include "iotcon-struct.h"

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
	struct icl_list_s *list;
} icl_val_list_s;

typedef struct {
	int type;
	struct icl_repr_s *repr;
} icl_val_repr_s;

iotcon_value_h icl_value_new_null();
iotcon_value_h icl_value_new_int(int val);
iotcon_value_h icl_value_new_bool(bool val);
iotcon_value_h icl_value_new_double(double val);
iotcon_value_h icl_value_new_str(const char *val);
iotcon_value_h icl_value_new_list(iotcon_list_h val);
iotcon_value_h icl_value_new_repr(iotcon_repr_h val);


int icl_value_get_int(iotcon_value_h value, int *val);
int icl_value_get_bool(iotcon_value_h value, bool *val);
int icl_value_get_double(iotcon_value_h value, double *val);
int icl_value_get_str(iotcon_value_h value, const char **val);
int icl_value_get_list(iotcon_value_h value, iotcon_list_h *list);
int icl_value_get_repr(iotcon_value_h value, iotcon_repr_h *repr);

void icl_value_free(gpointer data);

iotcon_value_h icl_value_clone(iotcon_value_h src);

#endif /* __IOT_CONNECTIVITY_MANAGER_LIBRARY_REPRESENTATION_VALUE_H__ */
