#include "stdafx_shared.h"
#include "pragma/model/animation/animation_event.h"
#include "pragma/model/animation/animation.h"

DLLNETWORK std::unordered_map<int32_t,std::string> ANIMATION_EVENT_NAMES = {
	{umath::to_integral(AnimationEvent::Type::EmitSound),"EVENT_EMITSOUND"},
	{umath::to_integral(AnimationEvent::Type::FootstepLeft),"EVENT_FOOTSTEP_LEFT"},
	{umath::to_integral(AnimationEvent::Type::FootstepRight),"EVENT_FOOTSTEP_RIGHT"}
};

void pragma::register_engine_animation_events()
{
	auto &reg = Animation::GetEventEnumRegister();
	for(auto i=std::underlying_type_t<AnimationEvent::Type>{0};i<umath::to_integral(AnimationEvent::Type::Count);++i)
	{
		auto it = ANIMATION_EVENT_NAMES.find(i);
		assert(it != ANIMATION_EVENT_NAMES.end());
		auto id = reg.RegisterEnum(it->second);
		assert(id == i);
	}
}
