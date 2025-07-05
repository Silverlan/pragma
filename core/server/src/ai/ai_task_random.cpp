// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/ai/ai_task_random.h"
#include "pragma/ai/ai_schedule.h"
#include <pragma/model/animation/activities.h>

using namespace pragma;

ai::BehaviorNode::Result ai::TaskRandom::Start(const Schedule *sched, pragma::SAIComponent &ent)
{
	BehaviorNode::Start(sched, ent);

	auto *paramChance = GetParameter(sched, umath::to_integral(Parameter::Chance));
	auto chance = (paramChance != nullptr) ? paramChance->GetFloat() : 0.5f;
	if(umath::random(0.f, 1.f) <= chance)
		return Result::Succeeded;
	return Result::Failed;
}

void ai::TaskRandom::Print(const Schedule *sched, std::ostream &o) const { o << "Random[" << GetChance() << "]"; }

float ai::TaskRandom::GetChance() const
{
	auto *p = GetParameter(umath::to_integral(Parameter::Chance));
	return (p != nullptr) ? p->GetFloat() : 0.f;
}

void ai::TaskRandom::SetChance(float chance) { SetParameter(umath::to_integral(Parameter::Chance), chance); }
