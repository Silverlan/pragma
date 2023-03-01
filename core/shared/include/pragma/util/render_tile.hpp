/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_RENDER_TILE_HPP__
#define __PRAGMA_RENDER_TILE_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/umat.h>

namespace pragma::rendering {
	struct DLLNETWORK Tile {
		Tile() = default;
		Tile(float x, float y, float w, float h);
		float x = 0.f;
		float y = 0.f;
		float w = 0.f;
		float h = 0.f;
	};
	DLLNETWORK Mat4 calc_tile_offset_matrix(const Tile &tile);
};

DLLNETWORK std::ostream &operator<<(std::ostream &str, const pragma::rendering::Tile &tile);

#endif
