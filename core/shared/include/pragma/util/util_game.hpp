/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __UTIL_GAME_HPP__
#define __UTIL_GAME_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_parallel_job.hpp>
#include <mathutil/uvec.h>
#include <vector>
#include <cinttypes>

class NetworkState;
struct Vertex;
namespace util
{
	constexpr inline double units_to_metres(double units) {return units *0.025;} // 1.75 (Avg. Height in meters) /72 (Player Height in units) ~= 0.025
	constexpr inline double metres_to_units(double meters) {return meters /0.025;}

	DLLNETWORK util::ParallelJob<std::vector<Vector2>&> generate_lightmap_uvs(NetworkState &nwState,uint32_t atlastWidth,uint32_t atlasHeight,const std::vector<Vertex> &verts,const std::vector<uint32_t> &tris);
};

#endif
