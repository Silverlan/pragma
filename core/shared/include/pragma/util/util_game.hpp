/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_GAME_HPP__
#define __UTIL_GAME_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_pragma.hpp>
#include <sharedutils/util_parallel_job.hpp>
#include <mathutil/uvec.h>
#include <vector>
#include <cinttypes>

class NetworkState;
struct Vertex;
namespace util
{
	DLLNETWORK util::ParallelJob<std::vector<Vector2>&> generate_lightmap_uvs(NetworkState &nwState,uint32_t atlastWidth,uint32_t atlasHeight,const std::vector<Vertex> &verts,const std::vector<uint32_t> &tris);
};

#endif
