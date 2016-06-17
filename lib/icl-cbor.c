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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <uuid/uuid.h>

#include "icl.h"
#include "icl-cbor.h"

#define ICL_CBOR_UUID_STR_LEN 36
#define ICL_CBOR_UUID_SIZE 16

int icl_cbor_create_svr_db(const char *path)
{
	int fd;
	int ret;
	const uint8_t cbor_binary[] = {
		0xa3, 0x63, 0x61, 0x63, 0x6c, 0x59, 0x02, 0x2a, 0xa2, 0x66, 0x61, 0x63, 0x6c, 0x69,
		0x73, 0x74, 0xa1, 0x64, 0x61, 0x63, 0x65, 0x73, 0x82, 0xa3, 0x6b, 0x73, 0x75, 0x62,
		0x6a, 0x65, 0x63, 0x74, 0x75, 0x75, 0x69, 0x64, 0x61, 0x2a, 0x69, 0x72, 0x65, 0x73,
		0x6f, 0x75, 0x72, 0x63, 0x65, 0x73, 0x87, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x68,
		0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x72, 0x65, 0x73, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62,
		0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x66,
		0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x64, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74,
		0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x66, 0x2f, 0x6f,
		0x69, 0x63, 0x2f, 0x70, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62,
		0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x6a, 0x2f, 0x6f, 0x69, 0x63,
		0x2f, 0x72, 0x65, 0x73, 0x2f, 0x64, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74,
		0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x70, 0x2f, 0x6f,
		0x69, 0x63, 0x2f, 0x72, 0x65, 0x73, 0x2f, 0x74, 0x79, 0x70, 0x65, 0x73, 0x2f, 0x64,
		0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0xa4,
		0x64, 0x68, 0x72, 0x65, 0x66, 0x6d, 0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x70, 0x72, 0x65,
		0x73, 0x65, 0x6e, 0x63, 0x65, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60,
		0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x6c, 0x2f, 0x6f, 0x69,
		0x63, 0x2f, 0x73, 0x65, 0x63, 0x2f, 0x76, 0x65, 0x72, 0x63, 0x72, 0x65, 0x6c, 0x60,
		0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0x6a, 0x70, 0x65, 0x72, 0x6d, 0x69,
		0x73, 0x73, 0x69, 0x6f, 0x6e, 0x02, 0xa3, 0x6b, 0x73, 0x75, 0x62, 0x6a, 0x65, 0x63,
		0x74, 0x75, 0x75, 0x69, 0x64, 0x61, 0x2a, 0x69, 0x72, 0x65, 0x73, 0x6f, 0x75, 0x72,
		0x63, 0x65, 0x73, 0x86, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x6d, 0x2f, 0x6f, 0x69,
		0x63, 0x2f, 0x73, 0x65, 0x63, 0x2f, 0x64, 0x6f, 0x78, 0x6d, 0x63, 0x72, 0x65, 0x6c,
		0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65,
		0x66, 0x6e, 0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x73, 0x65, 0x63, 0x2f, 0x70, 0x73, 0x74,
		0x61, 0x74, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66,
		0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x6c, 0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x73,
		0x65, 0x63, 0x2f, 0x61, 0x63, 0x6c, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74,
		0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x6d, 0x2f, 0x6f,
		0x69, 0x63, 0x2f, 0x73, 0x65, 0x63, 0x2f, 0x63, 0x72, 0x65, 0x64, 0x63, 0x72, 0x65,
		0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72,
		0x65, 0x66, 0x6e, 0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x73, 0x65, 0x63, 0x2f, 0x70, 0x63,
		0x6f, 0x6e, 0x66, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69,
		0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x71, 0x2f, 0x6f, 0x69, 0x63, 0x2f,
		0x73, 0x65, 0x63, 0x2f, 0x64, 0x70, 0x61, 0x69, 0x72, 0x69, 0x6e, 0x67, 0x63, 0x72,
		0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0x6a, 0x70, 0x65,
		0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x06, 0x6a, 0x72, 0x6f, 0x77, 0x6e,
		0x65, 0x72, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30,
		0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x65, 0x70, 0x73, 0x74, 0x61, 0x74, 0x58, 0x79, 0xa7, 0x64, 0x69, 0x73,
		0x6f, 0x70, 0xf4, 0x6a, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x75, 0x75, 0x69, 0x64,
		0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30,
		0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x6a, 0x72, 0x6f, 0x77,
		0x6e, 0x65, 0x72, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d,
		0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x62, 0x63, 0x6d, 0x02, 0x62, 0x74, 0x6d, 0x20, 0x62, 0x6f, 0x6d,
		0x03, 0x62, 0x73, 0x6d, 0x03, 0x64, 0x64, 0x6f, 0x78, 0x6d, 0x58, 0xb6, 0xa8, 0x64,
		0x6f, 0x78, 0x6d, 0x73, 0x81, 0x20, 0x66, 0x6f, 0x78, 0x6d, 0x73, 0x65, 0x6c, 0x20,
		0x63, 0x73, 0x63, 0x74, 0x01, 0x65, 0x6f, 0x77, 0x6e, 0x65, 0x64, 0xf4, 0x6a, 0x64,
		0x65, 0x76, 0x69, 0x63, 0x65, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30,
		0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x6c, 0x64, 0x65, 0x76, 0x6f, 0x77, 0x6e, 0x65, 0x72,
		0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30,
		0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x6a, 0x72, 0x6f, 0x77, 0x6e, 0x65, 0x72, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30,
		0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x63, 0x64, 0x70, 0x63, 0xf5};

	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (fd < 0) {
		ERR("open() Fail(%d)", errno);
		return IOTCON_ERROR_SYSTEM;
	}

	ret = write(fd, cbor_binary, sizeof(cbor_binary));
	if (ret <= 0) {
		ERR("write() Fail(%d)", errno);
		close(fd);
		return IOTCON_ERROR_SYSTEM;
	}
	close(fd);

	return IOTCON_ERROR_NONE;
}


int icl_cbor_create_pt_svr_db(const char *path)
{
	int i;
	int fd;
	int ret;
	uint8_t uuid[ICL_CBOR_UUID_SIZE];
	char uuid_str[ICL_CBOR_UUID_STR_LEN + 1];
	const int uuid_points[] = {356, 420, 485, 569, 620, 669};
	uint8_t cbor_binary[] = {
		0xbf, 0x63, 0x61, 0x63, 0x6c, 0x59, 0x01, 0x80, 0xa2, 0x66, 0x61, 0x63, 0x6c, 0x69,
		0x73, 0x74, 0xa1, 0x64, 0x61, 0x63, 0x65, 0x73, 0x82, 0xa3, 0x6b, 0x73, 0x75, 0x62,
		0x6a, 0x65, 0x63, 0x74, 0x75, 0x75, 0x69, 0x64, 0x61, 0x2a, 0x69, 0x72, 0x65, 0x73,
		0x6f, 0x75, 0x72, 0x63, 0x65, 0x73, 0x86, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x68,
		0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x72, 0x65, 0x73, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62,
		0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x66,
		0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x64, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74,
		0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x66, 0x2f, 0x6f,
		0x69, 0x63, 0x2f, 0x70, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62,
		0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x70, 0x2f, 0x6f, 0x69, 0x63,
		0x2f, 0x72, 0x65, 0x73, 0x2f, 0x74, 0x79, 0x70, 0x65, 0x73, 0x2f, 0x64, 0x63, 0x72,
		0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68,
		0x72, 0x65, 0x66, 0x67, 0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x61, 0x64, 0x63, 0x72, 0x65,
		0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72,
		0x65, 0x66, 0x6e, 0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x73, 0x65, 0x63, 0x2f, 0x61, 0x6d,
		0x61, 0x63, 0x6c, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69,
		0x66, 0x60, 0x6a, 0x70, 0x65, 0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x02,
		0xa3, 0x6b, 0x73, 0x75, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x75, 0x75, 0x69, 0x64, 0x61,
		0x2a, 0x69, 0x72, 0x65, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x73, 0x82, 0xa4, 0x64,
		0x68, 0x72, 0x65, 0x66, 0x6d, 0x2f, 0x6f, 0x69, 0x63, 0x2f, 0x73, 0x65, 0x63, 0x2f,
		0x64, 0x6f, 0x78, 0x6d, 0x63, 0x72, 0x65, 0x6c, 0x60, 0x62, 0x72, 0x74, 0x60, 0x62,
		0x69, 0x66, 0x60, 0xa4, 0x64, 0x68, 0x72, 0x65, 0x66, 0x6e, 0x2f, 0x6f, 0x69, 0x63,
		0x2f, 0x73, 0x65, 0x63, 0x2f, 0x70, 0x73, 0x74, 0x61, 0x74, 0x63, 0x72, 0x65, 0x6c,
		0x60, 0x62, 0x72, 0x74, 0x60, 0x62, 0x69, 0x66, 0x60, 0x6a, 0x70, 0x65, 0x72, 0x6d,
		0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x02, 0x6a, 0x72, 0x6f, 0x77, 0x6e, 0x65, 0x72,
		0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30,
		0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x65, 0x70, 0x73, 0x74, 0x61, 0x74, 0x58, 0x79, 0xa7, 0x64, 0x69, 0x73, 0x6f, 0x70,
		0xf5, 0x6a, 0x64, 0x65, 0x76, 0x69, 0x63, 0x65, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d,
		0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x62, 0x63, 0x6d, 0x00, 0x62, 0x74,
		0x6d, 0x00, 0x62, 0x6f, 0x6d, 0x03, 0x62, 0x73, 0x6d, 0x03, 0x6a, 0x72, 0x6f, 0x77,
		0x6e, 0x65, 0x72, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d,
		0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x64, 0x64, 0x6f, 0x78, 0x6d, 0x58, 0xb6, 0xa8, 0x64, 0x6f, 0x78,
		0x6d, 0x73, 0x81, 0x00, 0x66, 0x6f, 0x78, 0x6d, 0x73, 0x65, 0x6c, 0x00, 0x63, 0x73,
		0x63, 0x74, 0x01, 0x65, 0x6f, 0x77, 0x6e, 0x65, 0x64, 0xf5, 0x6a, 0x64, 0x65, 0x76,
		0x69, 0x63, 0x65, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d,
		0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x6c, 0x64, 0x65, 0x76, 0x6f, 0x77, 0x6e, 0x65, 0x72, 0x75, 0x75,
		0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30,
		0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x6a, 0x72,
		0x6f, 0x77, 0x6e, 0x65, 0x72, 0x75, 0x75, 0x69, 0x64, 0x78, 0x24, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30,
		0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x2d, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x30, 0x30, 0x63, 0x64, 0x70, 0x63, 0xf4, 0xff
	};

	uuid_generate(uuid);
	ret = snprintf(uuid_str, sizeof(uuid_str),
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
			uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	SECURE_DBG("uuid: %s", uuid_str);
	if (ICL_CBOR_UUID_STR_LEN != ret) {
		ERR("snprintf() Fail(%d)", ret);
		return IOTCON_ERROR_SYSTEM;
	}

	fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0660);
	if (fd < 0) {
		ERR("open() Fail(%d)", errno);
		return IOTCON_ERROR_SYSTEM;
	}

	for (i = 0 ; i < sizeof(uuid_points)/sizeof(int); i++)
		memcpy(cbor_binary + uuid_points[i], uuid_str, ICL_CBOR_UUID_STR_LEN);

	ret = write(fd, cbor_binary, sizeof(cbor_binary)/sizeof(uint8_t));
	if (ret <= 0) {
		ERR("write() Fail(%d)", errno);
		close(fd);
		return IOTCON_ERROR_SYSTEM;
	}

	close(fd);

	return IOTCON_ERROR_NONE;
}


