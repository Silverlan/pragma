// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.server;
import :ai.tasks.wait;

import :game;

using namespace pragma;

ai::BehaviorNode::Result ai::TaskWait::Start(const Schedule *sched, pragma::BaseAIComponent &ent)
{
	BehaviorNode::Start(sched, ent);
	auto *tParamMin = GetParameter(sched, pragma::math::to_integral(Parameter::MinWaitTime));
	auto *tParamMax = GetParameter(sched, pragma::math::to_integral(Parameter::MaxWaitTime));
	if(tParamMin == nullptr || tParamMax == nullptr || tParamMin->GetType() != ai::BehaviorParameter::Type::Float || tParamMax->GetType() != ai::BehaviorParameter::Type::Float)
		return Result::Failed;
	auto dur = pragma::math::random(tParamMin->GetFloat(), tParamMax->GetFloat());
	if(dur <= 0.f)
		return Result::Succeeded;
	m_tFinished = SGame::Get()->CurTime() + static_cast<double>(dur);
	return Result::Pending;
}

void ai::TaskWait::Print(const Schedule *sched, std::ostream &o) const
{
	auto *pMin = GetParameter(pragma::math::to_integral(Parameter::MinWaitTime));
	auto *pMax = GetParameter(pragma::math::to_integral(Parameter::MaxWaitTime));
	auto minTime = (pMin != nullptr) ? pMin->GetFloat() : 0.f;
	auto maxTime = (pMax != nullptr) ? pMax->GetFloat() : 0.f;
	o << "Wait[" << minTime << "][" << maxTime << "]";
}

ai::BehaviorNode::Result ai::TaskWait::Think(const Schedule *sched, pragma::BaseAIComponent &ent)
{
	auto r = BehaviorNode::Think(sched, ent);
	if(r != Result::Succeeded)
		return r;
	auto &tCur = SGame::Get()->CurTime();
	if(tCur < m_tFinished)
		return Result::Pending;
	return Result::Succeeded;
}

void ai::TaskWait::SetMinWaitTime(float t) { SetParameter(pragma::math::to_integral(Parameter::MinWaitTime), t); }
void ai::TaskWait::SetMaxWaitTime(float t) { SetParameter(pragma::math::to_integral(Parameter::MaxWaitTime), t); }
void ai::TaskWait::SetWaitTime(float tMin, float tMax)
{
	SetMinWaitTime(tMin);
	SetMaxWaitTime(tMax);
}
