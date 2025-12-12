// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation;

export import :model.animation.animation;
export import :model.animation.animation_event;
export import :model.animation.bone;
export import :model.animation.bone_list;
export import :model.animation.enums;
export import :model.animation.flex_animation;
export import :model.animation.frame;
export import :model.animation.meta_rig;
export import :model.animation.phoneme_map;
export import :model.animation.pose;
export import :model.animation.skeletal_animation;
export import :model.animation.skeleton;
export import :model.animation.vertex_animation;

export namespace pragma::math {
	DLLNETWORK Vector3 angular_velocity_to_linear(const Vector3 &refPos, const Vector3 &angVel, const Vector3 &tgtPos);
};
