/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/ai_task_debug.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/debug/debugoverlay.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/model/animation/activities.h>

using namespace pragma;

ai::BehaviorNode::Result ai::TaskDebugPrint::Start(const Schedule *sched, pragma::SAIComponent &ent)
{
	BehaviorNode::Start(sched, ent);
	std::string msg;
	if(GetDebugMessage(sched, msg) == false)
		return Result::Succeeded;
	Con::csv << "[AITask] " << msg << Con::endl;
	return Result::Succeeded;
}
bool ai::TaskDebugPrint::GetDebugMessage(const Schedule *sched, std::string &msg) const
{
	auto *param = GetParameter(sched, umath::to_integral(Parameter::Message));
	if(param == nullptr)
		return false;
	msg = param->ToString();
	return true;
}
void ai::TaskDebugPrint::SetMessage(const std::string &msg) { SetParameter(umath::to_integral(Parameter::Message), msg); }
void ai::TaskDebugPrint::Print(const Schedule *sched, std::ostream &o) const
{
	std::string msg;
	o << "DebugPrint[" << ((GetDebugMessage(sched, msg) == false) ? "null" : msg) << "]";
}

////////////////////

ai::BehaviorNode::Result ai::TaskDebugDrawText::Start(const Schedule *sched, pragma::SAIComponent &aiComponent)
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
	SDebugRenderer::DrawText(msg, pos, 0.5f, Color::White, 1.f);
	return Result::Succeeded;
}
void ai::TaskDebugDrawText::SetMessage(const std::string &msg) { SetParameter(umath::to_integral(Parameter::Message), msg); }
bool ai::TaskDebugDrawText::GetDebugMessage(const Schedule *sched, std::string &msg) const
{
	auto *param = GetParameter(sched, umath::to_integral(Parameter::Message));
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
