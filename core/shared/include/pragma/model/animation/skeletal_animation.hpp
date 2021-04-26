/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SKELETAL_ANIMATION_HPP__
#define __SKELETAL_ANIMATION_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>

enum class Activity : uint16_t;
using BoneId = uint16_t;
class Skeleton;
namespace util {class EnumRegister;};
namespace pragma::animation {class Animation; struct AnimationChannel; struct AnimationSlice; class AnimatedPose;};
namespace pragma::animation::skeletal
{
	static constexpr std::string_view SK_ANIMATED_COMPONENT_NAME = "sk_animated";
	struct DLLNETWORK AnimBoneChannelDesc
	{
		static constexpr auto INVALID_CHANNEL = std::numeric_limits<uint32_t>::max();
		uint32_t positionChannel = INVALID_CHANNEL;
		uint32_t rotationChannel = INVALID_CHANNEL;
		uint32_t scaleChannel = INVALID_CHANNEL;
	};
	using BoneChannelMap = std::unordered_map<BoneId,AnimBoneChannelDesc>;

	util::EnumRegister &get_activity_enum_register();
	Activity get_activity(const Animation &anim);
	void set_activity(Animation &anim,Activity act);
	uint8_t get_activity_weight(const Animation &anim);
	void set_activity_weight(Animation &anim,uint8_t weight);

	std::pair<Vector3,Vector3> get_render_bounds(const Animation &anim);
	void set_render_bounds(Animation &anim,const Vector3 &min,const Vector3 &max);

	BoneChannelMap get_bone_channel_map(const Animation &animation,const Skeleton &skeleton);
	void animation_slice_to_animated_pose(const BoneChannelMap &boneChannelMap,const AnimationSlice &slice,AnimatedPose &pose);

	void interpolate_animated_poses(const AnimatedPose &pose0,const AnimatedPose &pose1,AnimatedPose &poseDst,float f);

	bool is_bone_position_channel(const AnimationChannel &channel);
	bool is_bone_rotation_channel(const AnimationChannel &channel);
	bool is_bone_scale_channel(const AnimationChannel &channel);

	void translate(Animation &anim,const Vector3 &translation);
	void rotate(Animation &anim,const Quat &rotation);
	void scale(Animation &anim,const Vector3 &scale);
};

#endif
