// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:math.frustum;

export import std.compat;

export namespace pragma::math {
	enum class FrustumPlane : uint32_t {
		Left = 0,
		Right,
		Top,
		Bottom,
		Near,
		Far,

		Count
	};

	enum class FrustumPoint : uint32_t { FarBottomLeft = 0, FarTopLeft, FarTopRight, FarBottomRight, NearBottomLeft, NearTopLeft, NearTopRight, NearBottomRight };
};
