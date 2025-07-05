// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
