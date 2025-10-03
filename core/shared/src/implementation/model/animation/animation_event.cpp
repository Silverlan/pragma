// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "cassert"

#include "pragma/networkdefinitions.h"

module pragma.shared;

import :model.animation.animation_event;

DLLNETWORK std::unordered_map<int32_t, std::string> ANIMATION_EVENT_NAMES
  = {{umath::to_integral(AnimationEvent::Type::EmitSound), "EVENT_EMITSOUND"}, {umath::to_integral(AnimationEvent::Type::FootstepLeft), "EVENT_FOOTSTEP_LEFT"}, {umath::to_integral(AnimationEvent::Type::FootstepRight), "EVENT_FOOTSTEP_RIGHT"}};

void pragma::register_engine_animation_events()
{
	auto &reg = pragma::animation::Animation::GetEventEnumRegister();
	for(auto i = std::underlying_type_t<AnimationEvent::Type> {0}; i < umath::to_integral(AnimationEvent::Type::Count); ++i) {
		auto it = ANIMATION_EVENT_NAMES.find(i);
		assert(it != ANIMATION_EVENT_NAMES.end());
		auto id = reg.RegisterEnum(it->second);
		assert(id == i);
	}
}
