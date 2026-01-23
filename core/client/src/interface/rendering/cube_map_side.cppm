// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.cube_map_side;

export import std.compat;

export namespace pragma::rendering {
	enum class CubeMapSide : uint32_t { Left = 0, Right = 1, Top = 2, Bottom = 3, Front = 4, Back = 5 };
}
