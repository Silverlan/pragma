/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/ai_task_wait.h"
#include "pragma/ai/ai_schedule.h"
#include <pragma/model/animation/activities.h>

using namespace pragma;

extern DLLSERVER SGame *s_game;

ai::BehaviorNode::Result ai::TaskWait::Start(const Schedule *sched,pragma::SAIComponent &ent)
{
	BehaviorNode::Start(sched,ent);
	auto *tParamMin = GetParameter(sched,umath::to_integral(Parameter::MinWaitTime));
	auto *tParamMax = GetParameter(sched,umath::to_integral(Parameter::MaxWaitTime));
	if(tParamMin == nullptr || tParamMax == nullptr || tParamMin->GetType() != ai::BehaviorParameter::Type::Float || tParamMax->GetType() != ai::BehaviorParameter::Type::Float)
		return Result::Failed;
	auto dur = umath::random(tParamMin->GetFloat(),tParamMax->GetFloat());
	if(dur <= 0.f)
		return Result::Succeeded;
	m_tFinished = s_game->CurTime() +static_cast<double>(dur);
	return Result::Pending;
}

void ai::TaskWait::Print(const Schedule *sched,std::ostream &o) const
{
	auto *pMin = GetParameter(umath::to_integral(Parameter::MinWaitTime));
	auto *pMax = GetParameter(umath::to_integral(Parameter::MaxWaitTime));
	auto minTime = (pMin != nullptr) ? pMin->GetFloat() : 0.f;
	auto maxTime = (pMax != nullptr) ? pMax->GetFloat() : 0.f;
	o<<"Wait["<<minTime<<"]["<<maxTime<<"]";
}

ai::BehaviorNode::Result ai::TaskWait::Think(const Schedule *sched,pragma::SAIComponent &ent)
{
	auto r = BehaviorNode::Think(sched,ent);
	if(r != Result::Succeeded)
		return r;
	auto &tCur = s_game->CurTime();
	if(tCur < m_tFinished)
		return Result::Pending;
	return Result::Succeeded;
}

void ai::TaskWait::SetMinWaitTime(float t)
{
	SetParameter(umath::to_integral(Parameter::MinWaitTime),t);
}
void ai::TaskWait::SetMaxWaitTime(float t)
{
	SetParameter(umath::to_integral(Parameter::MaxWaitTime),t);
}
void ai::TaskWait::SetWaitTime(float tMin,float tMax)
{
	SetMinWaitTime(tMin);
	SetMaxWaitTime(tMax);
}
