// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
