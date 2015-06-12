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
#include <OCApi.h>
#include <OCPlatform.h>

extern "C" {
#include <glib.h>
#include "iotcon-struct.h"
#include "ic-common.h"
#include "ic-utils.h"
#include "icd.h"
#include "icd-ioty-repr.h"
}

using namespace std;
using namespace OC;

int icd_ioty_repr_parse_json(const char *repr_json, OCRepresentation &ocRep)
{
	int ret = IOTCON_ERROR_NONE;
	MessageContainer info;

	try {
		info.setJSONRepresentation(repr_json);

		const vector<OCRepresentation> &reps = info.representations();
		if (0 < reps.size()) {
			vector<OCRepresentation>::const_iterator itr = reps.begin();
			vector<OCRepresentation>::const_iterator back = reps.end();
			ocRep = *itr;
			++itr;

			for (; itr != back; ++itr)
				ocRep.addChild(*itr);
		} else {
			ERR("Invalid parameter(%s)", repr_json);
			ret = IOTCON_ERROR_INVALID_PARAMETER;
		}
	} catch (exception &e) {
		ERR("setJSONRepresentation() Fail(%s)", e.what());
		ret = IOTCON_ERROR_INVALID_PARAMETER;
	}

	return ret;
}


int icd_ioty_repr_generate_json(OCRepresentation ocRep, char **json)
{
	RETV_IF(NULL == json, IOTCON_ERROR_INVALID_PARAMETER);

	*json = ic_utils_strdup(ocRep.getJSONRepresentation().c_str());
	if (NULL == *json) {
		ERR("ic_utils_strdup() Fail");
		return IOTCON_ERROR_INVALID_PARAMETER;
	}

	return IOTCON_ERROR_NONE;
}


int icd_ioty_repr_generate_gvariant_builder(const OCRepresentation &ocRep,
		GVariantBuilder **builder)
{
	int ret;
	unsigned int i;
	char *repr_json;
	OCRepresentation ocChild;

	*builder = g_variant_builder_new(G_VARIANT_TYPE("as"));

	DBG("numberOfAttributes : %d", ocRep.numberOfAttributes());
	ret = icd_ioty_repr_generate_json(ocRep, &repr_json);
	if (IOTCON_ERROR_NONE != ret) {
		ERR("icd_ioty_repr_generate_json() Fail(%d)", ret);
		g_variant_builder_unref(*builder);
		return ret;
	}
	g_variant_builder_add(*builder, "s", repr_json);
	free(repr_json);

	vector<OCRepresentation> childList = ocRep.getChildren();

	for (i = 0; i < childList.size(); i++) {
		ocChild = childList.at(i);
		ret = icd_ioty_repr_generate_json(ocChild, &repr_json);
		if (IOTCON_ERROR_NONE != ret) {
			ERR("icd_ioty_repr_generate_json() Fail(%d)", ret);
			g_variant_builder_unref(*builder);
			return ret;
		}
		g_variant_builder_add(*builder, "s", repr_json);
		free(repr_json);
	}

	return IOTCON_ERROR_NONE;
}
