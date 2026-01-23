// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.joint_info;

export import :model.animation.bone;

export namespace pragma::physics {
	enum class JointType : uint8_t { None = 0, Fixed, BallSocket, Hinge, Slider, ConeTwist, DOF };

	struct DLLNETWORK JointInfo {
		JointInfo(JointType type, animation::BoneId child, animation::BoneId parent) : type(type), parent(parent), child(child), collide(false) {}
		JointInfo() : JointInfo(JointType::None, 0, 0) {}
		JointType type;
		animation::BoneId parent;
		animation::BoneId child;
		bool collide;
		std::unordered_map<std::string, std::string> args;

		bool operator==(const JointInfo &other) const { return type == other.type && parent == other.parent && child == other.child && collide == other.collide && args == other.args; }
		bool operator!=(const JointInfo &other) const { return !operator==(other); }
	};
};
