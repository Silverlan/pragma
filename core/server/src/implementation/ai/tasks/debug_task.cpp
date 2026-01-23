// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.debug;

import :debug;
import :entities.components;

#undef DrawText

using namespace pragma;

ai::BehaviorNode::Result ai::TaskDebugPrint::Start(const Schedule *sched, BaseAIComponent &ent)
{
	BehaviorNode::Start(sched, ent);
	std::string msg;
	if(GetDebugMessage(sched, msg) == false)
		return Result::Succeeded;
	Con::CSV << "[AITask] " << msg << Con::endl;
	return Result::Succeeded;
}
bool ai::TaskDebugPrint::GetDebugMessage(const Schedule *sched, std::string &msg) const
{
	auto *param = GetParameter(sched, math::to_integral(Parameter::Message));
	if(param == nullptr)
		return false;
	msg = param->ToString();
	return true;
}
void ai::TaskDebugPrint::SetMessage(const std::string &msg) { SetParameter(math::to_integral(Parameter::Message), msg); }
void ai::TaskDebugPrint::Print(const Schedule *sched, std::ostream &o) const
{
	std::string msg;
	o << "DebugPrint[" << ((GetDebugMessage(sched, msg) == false) ? "null" : msg) << "]";
}

////////////////////

ai::BehaviorNode::Result ai::TaskDebugDrawText::Start(const Schedule *sched, BaseAIComponent &aiComponent)
{
	BehaviorNode::Start(sched, aiComponent);
	std::string msg;
	if(GetDebugMessage(sched, msg) == false)
		return Result::Succeeded;
	auto &ent = aiComponent.GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return Result::Failed;
	auto pos = pTrComponent->GetPosition();
	Vector3 min {};
	Vector3 max {};
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		pPhysComponent->GetCollisionBounds(&min, &max);
	pos.y += max.y;
	debug::SDebugRenderer::DrawText(msg, pos, 0.5f, colors::White, 1.f);
	return Result::Succeeded;
}
void ai::TaskDebugDrawText::SetMessage(const std::string &msg) { SetParameter(math::to_integral(Parameter::Message), msg); }
bool ai::TaskDebugDrawText::GetDebugMessage(const Schedule *sched, std::string &msg) const
{
	auto *param = GetParameter(sched, math::to_integral(Parameter::Message));
	if(param == nullptr)
		return false;
	msg = param->ToString();
	return true;
}
void ai::TaskDebugDrawText::Print(const Schedule *sched, std::ostream &o) const
{
	BehaviorNode::Print(sched, o);
	std::string msg;
	o << "DebugDrawText[" << ((GetDebugMessage(sched, msg) == false) ? "null" : msg) << "]";
}
