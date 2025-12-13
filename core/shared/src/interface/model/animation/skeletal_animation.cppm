// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.skeletal_animation;

import panima;

export import :model.animation.animation;
export import :model.animation.pose;

export {
	namespace pragma::animation::skeletal {
		constexpr std::string_view SK_ANIMATED_COMPONENT_NAME = "sk_animated";
		struct DLLNETWORK AnimBoneChannelDesc {
			static constexpr auto INVALID_CHANNEL = std::numeric_limits<uint32_t>::max();
			uint32_t positionChannel = INVALID_CHANNEL;
			uint32_t rotationChannel = INVALID_CHANNEL;
			uint32_t scaleChannel = INVALID_CHANNEL;
		};
		using BoneChannelMap = std::unordered_map<BoneId, AnimBoneChannelDesc>;

		util::EnumRegister &get_activity_enum_register();
		Activity get_activity(const panima::Animation &anim);
		void set_activity(panima::Animation &anim, Activity act);
		uint8_t get_activity_weight(const panima::Animation &anim);
		void set_activity_weight(panima::Animation &anim, uint8_t weight);

		std::pair<Vector3, Vector3> get_render_bounds(const panima::Animation &anim);
		void set_render_bounds(panima::Animation &anim, const Vector3 &min, const Vector3 &max);

		BoneChannelMap get_bone_channel_map(const panima::Animation &animation, const Skeleton &skeleton);
		void animation_slice_to_animated_pose(const BoneChannelMap &boneChannelMap, const panima::Slice &slice, Pose &pose);

		void interpolate_animated_poses(const Pose &pose0, const Pose &pose1, Pose &poseDst, float f);

		bool is_bone_position_channel(const panima::Channel &channel);
		bool is_bone_rotation_channel(const panima::Channel &channel);
		bool is_bone_scale_channel(const panima::Channel &channel);

		void translate(panima::Animation &anim, const Vector3 &translation);
		void rotate(panima::Animation &anim, const Quat &rotation);
		void scale(panima::Animation &anim, const Vector3 &scale);
	};
};
