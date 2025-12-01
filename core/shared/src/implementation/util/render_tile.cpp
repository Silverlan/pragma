// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.render_tile;

pragma::rendering::Tile::Tile(float x, float y, float w, float h) : x {x}, y {y}, w {w}, h {h} {}

Mat4 pragma::rendering::calc_tile_offset_matrix(const Tile &tile)
{
	auto x_clip = 2.f * tile.x - 1;
	auto y_clip = 2.f * tile.y - 1;
	auto w_clip = 2.f * tile.w - 1;
	auto h_clip = 2.f * tile.h - 1;

	Mat4 m0 {1.f};
	auto scale = glm::gtc::scale(m0, Vector3 {2.f / w_clip, 2.f / h_clip, 1.f});

	Mat4 m1 {1.f};
	auto translate = glm::gtc::translate(m1, Vector3 {-(x_clip + 0.5f * w_clip), -(y_clip + 0.5f * h_clip), 0.f});
	return scale * translate;
}

std::ostream &operator<<(std::ostream &str, const pragma::rendering::Tile &tile)
{
	str << "RenderTile[" << tile.x << "][" << tile.y << "][" << tile.w << "][" << tile.h << "]";
	return str;
}
