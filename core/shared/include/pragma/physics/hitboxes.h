/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __HITBOXES_H__
#define __HITBOXES_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>
#include <limits>

enum class DLLNETWORK HitGroup : uint32_t { Generic = 0, Head = 1, Chest = 2, Stomach = 3, LeftArm = 4, RightArm = 5, LeftLeg = 6, RightLeg = 7, Gear = 8, Tail = 9, Invalid = std::numeric_limits<std::underlying_type_t<HitGroup>>::max() };

struct DLLNETWORK Hitbox {
	Hitbox(HitGroup _group = HitGroup::Invalid, const Vector3 &_min = {}, const Vector3 &_max = {}) : group(_group), min(_min), max(_max) {}
	HitGroup group;
	Vector3 min;
	Vector3 max;

	bool operator==(const Hitbox &other) const { return group == other.group && uvec::cmp(min, other.min) && uvec::cmp(max, other.max); }
	bool operator!=(const Hitbox &other) const { return !operator==(other); }
};

#endif
