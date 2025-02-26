/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SKELETAL_ANIMATION_HPP__
#define __SKELETAL_ANIMATION_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include <cinttypes>

import panima;

enum class Activity : uint16_t;
namespace util {
	class EnumRegister;
};
namespace pragma::animation {
	using BoneId = uint16_t;
};
namespace pragma::animation::skeletal {
	static constexpr std::string_view SK_ANIMATED_COMPONENT_NAME = "sk_animated";
	struct DLLNETWORK AnimBoneChannelDesc {
		static constexpr auto INVALID_CHANNEL = std::numeric_limits<uint32_t>::max();
		uint32_t positionChannel = INVALID_CHANNEL;
		uint32_t rotationChannel = INVALID_CHANNEL;
		uint32_t scaleChannel = INVALID_CHANNEL;
	};
	using BoneChannelMap = std::unordered_map<pragma::animation::BoneId, AnimBoneChannelDesc>;

	util::EnumRegister &get_activity_enum_register();
	Activity get_activity(const panima::Animation &anim);
	void set_activity(panima::Animation &anim, Activity act);
	uint8_t get_activity_weight(const panima::Animation &anim);
	void set_activity_weight(panima::Animation &anim, uint8_t weight);

	std::pair<Vector3, Vector3> get_render_bounds(const panima::Animation &anim);
	void set_render_bounds(panima::Animation &anim, const Vector3 &min, const Vector3 &max);

	BoneChannelMap get_bone_channel_map(const panima::Animation &animation, const pragma::animation::Skeleton &skeleton);
	void animation_slice_to_animated_pose(const BoneChannelMap &boneChannelMap, const panima::Slice &slice, pragma::animation::Pose &pose);

	void interpolate_animated_poses(const pragma::animation::Pose &pose0, const pragma::animation::Pose &pose1, pragma::animation::Pose &poseDst, float f);

	bool is_bone_position_channel(const panima::Channel &channel);
	bool is_bone_rotation_channel(const panima::Channel &channel);
	bool is_bone_scale_channel(const panima::Channel &channel);

	void translate(panima::Animation &anim, const Vector3 &translation);
	void rotate(panima::Animation &anim, const Quat &rotation);
	void scale(panima::Animation &anim, const Vector3 &scale);
};

#endif
