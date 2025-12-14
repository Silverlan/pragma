// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include <cassert>

module pragma.shared;

import :model.animation.animation_event;

DLLNETWORK std::unordered_map<int32_t, std::string> ANIMATION_EVENT_NAMES
  = {{pragma::math::to_integral(pragma::AnimationEvent::Type::EmitSound), "EVENT_EMITSOUND"}, {pragma::math::to_integral(pragma::AnimationEvent::Type::FootstepLeft), "EVENT_FOOTSTEP_LEFT"}, {pragma::math::to_integral(pragma::AnimationEvent::Type::FootstepRight), "EVENT_FOOTSTEP_RIGHT"}};

void pragma::register_engine_animation_events()
{
	auto &reg = animation::Animation::GetEventEnumRegister();
	for(auto i = std::underlying_type_t<AnimationEvent::Type> {0}; i < math::to_integral(AnimationEvent::Type::Count); ++i) {
		auto it = ANIMATION_EVENT_NAMES.find(i);
		assert(it != ANIMATION_EVENT_NAMES.end());
		auto id = reg.RegisterEnum(it->second);
		assert(id == i);
	}
}
