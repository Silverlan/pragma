/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __JOINTINFO_H__
#define __JOINTINFO_H__

#include "pragma/networkdefinitions.h"
#include "pragma/model/animation/bone.hpp"

enum class JointType : uint8_t { None = 0, Fixed, BallSocket, Hinge, Slider, ConeTwist, DOF };

namespace pragma::animation {
	using BoneId = uint16_t;
};
struct DLLNETWORK JointInfo {
	JointInfo(JointType type, pragma::animation::BoneId child, pragma::animation::BoneId parent) : type(type), parent(parent), child(child), collide(false) {}
	JointInfo() : JointInfo(JointType::None, 0, 0) {}
	JointType type;
	pragma::animation::BoneId parent;
	pragma::animation::BoneId child;
	bool collide;
	std::unordered_map<std::string, std::string> args;

	bool operator==(const JointInfo &other) const { return type == other.type && parent == other.parent && child == other.child && collide == other.collide && args == other.args; }
	bool operator!=(const JointInfo &other) const { return !operator==(other); }
};

#endif
