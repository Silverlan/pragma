/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/render_tile.hpp"

pragma::rendering::Tile::Tile(float x, float y, float w, float h) : x {x}, y {y}, w {w}, h {h} {}

Mat4 pragma::rendering::calc_tile_offset_matrix(const Tile &tile)
{
	auto x_clip = 2.f * tile.x - 1;
	auto y_clip = 2.f * tile.y - 1;
	auto w_clip = 2.f * tile.w - 1;
	auto h_clip = 2.f * tile.h - 1;

	Mat4 m0 {1.f};
	auto scale = glm::scale(m0, Vector3 {2.f / w_clip, 2.f / h_clip, 1.f});

	Mat4 m1 {1.f};
	auto translate = glm::translate(m1, Vector3 {-(x_clip + 0.5f * w_clip), -(y_clip + 0.5f * h_clip), 0.f});
	return scale * translate;
}

std::ostream &operator<<(std::ostream &str, const pragma::rendering::Tile &tile)
{
	str << "RenderTile[" << tile.x << "][" << tile.y << "][" << tile.w << "][" << tile.h << "]";
	return str;
}
