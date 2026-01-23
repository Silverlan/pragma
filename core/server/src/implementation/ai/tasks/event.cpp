// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.event;

import :entities.components;

using namespace pragma;

void ai::TaskEvent::Print(const Schedule *sched, std::ostream &o) const
{
	auto *paramEvent = GetParameter(math::to_integral(Parameter::EventId));
	auto eventId = (paramEvent != nullptr) ? paramEvent->GetInt() : -1;
	o << "Event[" << eventId << "]";
	uint32_t argIdx = math::to_integral(Parameter::EventArgStart);
	auto *p = GetParameter(argIdx);
	while(p != nullptr) {
		o << "[" << p->GetString() << "]";
		p = GetParameter(argIdx++);
	}
}
ai::BehaviorNode::Result ai::TaskEvent::Start(const Schedule *sched, BaseAIComponent &aiComponent)
{
	BehaviorNode::Start(sched, aiComponent);

	auto *paramEventId = GetParameter(sched, math::to_integral(Parameter::EventId));
	if(paramEventId != nullptr) {
		AnimationEvent ev {};
		ev.eventID = static_cast<AnimationEvent::Type>(paramEventId->GetInt());
		uint32_t argIdx = math::to_integral(Parameter::EventArgStart);
		auto *paramArg = GetParameter(sched, argIdx);
		while(paramArg != nullptr) {
			auto *str = paramArg->GetString();
			if(str != nullptr)
				ev.arguments.push_back(*str);
			paramArg = GetParameter(sched, ++argIdx);
		}
		auto pAnimComponent = aiComponent.GetEntity().GetAnimatedComponent();
		if(pAnimComponent.valid())
			pAnimComponent->InjectAnimationEvent(ev);
	}
	return Result::Succeeded;
}

void ai::TaskEvent::SetEventId(AnimationEvent::Type eventId) { SetParameter(math::to_integral(Parameter::EventId), static_cast<int32_t>(eventId)); }
void ai::TaskEvent::SetEventArgument(uint32_t argIdx, const std::string &arg) { SetParameter(math::to_integral(Parameter::EventArgStart) + argIdx, arg); }
