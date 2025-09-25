// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>

export module pragma.shared:math.frustum;

export {
	enum class FrustumPlane : uint32_t {
		Left,
		Right,
		Top,
		Bottom,
		Near,
		Far,

		Count
	};

	enum class FrustumPoint : uint32_t { FarBottomLeft = 0, FarTopLeft, FarTopRight, FarBottomRight, NearBottomLeft, NearTopLeft, NearTopRight, NearBottomRight };
};
