/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/ai_task_event.hpp"
#include "pragma/ai/ai_schedule.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include <pragma/entities/components/base_animated_component.hpp>
#include <pragma/model/animation/activities.h>

using namespace pragma;

void ai::TaskEvent::Print(const Schedule *sched,std::ostream &o) const
{
	auto *paramEvent = GetParameter(umath::to_integral(Parameter::EventId));
	auto eventId = (paramEvent != nullptr) ? paramEvent->GetInt() : -1;
	o<<"Event["<<eventId<<"]";
	uint32_t argIdx = umath::to_integral(Parameter::EventArgStart);
	auto *p = GetParameter(argIdx);
	while(p != nullptr)
	{
		o<<"["<<p->GetString()<<"]";
		p = GetParameter(argIdx++);
	}
}
ai::BehaviorNode::Result ai::TaskEvent::Start(const Schedule *sched,pragma::SAIComponent &aiComponent)
{
	BehaviorNode::Start(sched,aiComponent);

	auto *paramEventId = GetParameter(sched,umath::to_integral(Parameter::EventId));
	if(paramEventId != nullptr)
	{
		AnimationEvent ev {};
		ev.eventID = static_cast<AnimationEvent::Type>(paramEventId->GetInt());
		uint32_t argIdx = umath::to_integral(Parameter::EventArgStart);
		auto *paramArg = GetParameter(sched,argIdx);
		while(paramArg != nullptr)
		{
			auto *str = paramArg->GetString();
			if(str != nullptr)
				ev.arguments.push_back(*str);
			paramArg = GetParameter(sched,++argIdx);
		}
		auto pAnimComponent = aiComponent.GetEntity().GetAnimatedComponent();
		if(pAnimComponent.valid())
			pAnimComponent->InjectAnimationEvent(ev);
	}
	return Result::Succeeded;
}

void ai::TaskEvent::SetEventId(AnimationEvent::Type eventId)
{
	SetParameter(umath::to_integral(Parameter::EventId),static_cast<int32_t>(eventId));
}
void ai::TaskEvent::SetEventArgument(uint32_t argIdx,const std::string &arg)
{
	SetParameter(umath::to_integral(Parameter::EventArgStart) +argIdx,arg);
}
