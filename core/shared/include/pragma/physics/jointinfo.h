// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
