/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __UTIL_BVH_HPP__
#define __UTIL_BVH_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/bvh_data.hpp"
#include <mathutil/uvec.h>

namespace pragma {
	namespace bvh {
		DLLNETWORK std::unique_ptr<MeshBvhTree> create_bvh_data(std::vector<Primitive> &&triangles);
	};
};

#endif
