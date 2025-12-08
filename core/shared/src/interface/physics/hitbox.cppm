// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.hitbox;

export import pragma.math;

export namespace pragma::physics {
	enum class HitGroup : uint32_t { Generic = 0, Head, Chest, Stomach, LeftArm, RightArm, LeftLeg, RightLeg, Gear, Tail, Invalid = std::numeric_limits<std::underlying_type_t<HitGroup>>::max() };

	struct DLLNETWORK Hitbox {
		Hitbox(HitGroup _group = HitGroup::Invalid, const Vector3 &_min = {}, const Vector3 &_max = {}) : group(_group), min(_min), max(_max) {}
		HitGroup group;
		Vector3 min;
		Vector3 max;

		bool operator==(const Hitbox &other) const { return group == other.group && uvec::cmp(min, other.min) && uvec::cmp(max, other.max); }
		bool operator!=(const Hitbox &other) const { return !operator==(other); }
	};
};
