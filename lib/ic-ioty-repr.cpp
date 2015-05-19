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

#include "iotcon.h"
extern "C" {
#include "ic-common.h"
#include "ic-utils.h"
#include "ic-repr.h"
#include "ic.h"
}
#include "ic-ioty-repr.h"

using namespace OC;
using namespace std;

static iotcon_repr_h _ic_ioty_repr_create_repr(const OCRepresentation& ocRep)
{
	FN_CALL;
	// incoming json format example : {"href":"/a/address","rep":{"text":"Hello world"}}
	string jsonStr = ocRep.getJSONRepresentation();
	iotcon_repr_h repr = ic_repr_parse_json(jsonStr.c_str());

	string uriStr = ocRep.getUri();
	if (!uriStr.empty())
		iotcon_repr_set_uri(repr, uriStr.c_str());

	return repr;
}

static iotcon_repr_h _ic_ioty_repr_create_parent(const OCRepresentation& ocRep)
{
	FN_CALL;
	return _ic_ioty_repr_create_repr(ocRep);
}

static iotcon_repr_h _ic_ioty_repr_create_child(const OCRepresentation& ocRep)
{
	FN_CALL;
	return _ic_ioty_repr_create_repr(ocRep);
}


/*
 * A general input : {oc:[{"href":"/a/parent","rep":{"string":"Hello","intlist":[1,2,3]},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}},
 * 						{"href":"/a/child","rep":{"string":"World","double_val":5.7},
 * 						"prop":{"rt":["core.light"],"if":["oc.mi.def"]}}]}
 */
iotcon_repr_h ic_ioty_repr_generate_repr(const OCRepresentation& ocRep)
{
	FN_CALL;
	unsigned int i = 0;
	OCRepresentation ocChild;

	iotcon_repr_h repr_parent = _ic_ioty_repr_create_parent(ocRep);
	if (NULL == repr_parent) {
		ERR("_ic_ioty_repr_create_parent() Fail");
		iotcon_repr_free(repr_parent);
		return NULL;
	}

	vector<OCRepresentation> childList = ocRep.getChildren();

	for (i = 0; i < childList.size(); i++) {
		ocChild = childList.at(i);
		iotcon_repr_h repr_child = _ic_ioty_repr_create_child(ocChild);
		if (NULL == repr_child) {
			ERR("_ic_ioty_repr_create_child() Fail");
			iotcon_repr_free(repr_parent);
			return NULL;
		}

		iotcon_repr_append_child(repr_parent, repr_child);
	}

	return repr_parent;
}

OCRepresentation ic_ioty_repr_parse(iotcon_repr_h repr)
{
	FN_CALL;
	OCRepresentation ocRep;
	MessageContainer info;

	/* TODO: It's better that iotcon_repr_h is changed to OCRepresentation at once. */
	char *repr_json = ic_repr_generate_json(repr, false);

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
		}
		else {
			ERR("Invalid parameter(%s)", repr_json);
		}
	} catch (exception &e) {
		ERR("setJSONRepresentation() Fail(%s)", e.what());
	}

	free(repr_json);
	return ocRep;
}

