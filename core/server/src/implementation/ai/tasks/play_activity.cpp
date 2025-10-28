// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server.ai.tasks.play_activity;

import pragma.server.entities.components;

using namespace pragma;

int32_t ai::TaskPlayActivity::SelectAnimation(const Schedule *sched, pragma::BaseAIComponent &ent, uint8_t paramId)
{
	auto *param = GetParameter(sched, paramId);
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		return -1;
	auto activity = static_cast<pragma::Activity>(param->GetInt());
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->SelectTranslatedAnimation(activity) : -1;
}

void ai::TaskPlayActivity::SetActivity(pragma::Activity activity) { SetParameter(umath::to_integral(Parameter::Activity), umath::to_integral(activity)); }
void ai::TaskPlayActivity::SetFaceTarget(const Vector3 &target) { SetParameter(umath::to_integral(Parameter::FaceTarget), target); }
void ai::TaskPlayActivity::SetFaceTarget(pragma::ecs::BaseEntity &target) { SetParameter(umath::to_integral(Parameter::FaceTarget), &target); }

void ai::TaskPlayActivity::Print(const Schedule *sched, std::ostream &o) const
{
	o << "PlayActivity[";
	auto *param = GetParameter(sched, umath::to_integral(Parameter::Activity));
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		o << "Nothing";
	else {
		auto actId = param->GetInt();
		auto it = ACTIVITY_NAMES.find(actId);
		if(it != ACTIVITY_NAMES.end())
			o << it->second;
		else
			o << actId;
	}
	o << "]";
}

////////////////////////////////////

void ai::TaskPlayLayeredActivity::Print(const Schedule *sched, std::ostream &o) const
{
	auto *pSlot = GetParameter(umath::to_integral(Parameter::AnimationSlot));
	o << "PlayLayeredActivity[";
	auto *param = GetParameter(sched, umath::to_integral(Parameter::Activity));
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		o << "Nothing";
	else {
		auto actId = param->GetInt();
		auto it = ACTIVITY_NAMES.find(actId);
		if(it != ACTIVITY_NAMES.end())
			o << it->second;
		else
			o << actId;
	}
	o << "][";
	if(pSlot != nullptr && pSlot->GetType() == pragma::ai::BehaviorParameter::Type::Int)
		o << pSlot->GetInt();
	else
		o << "NULL";
	o << "]";
}
void ai::TaskPlayLayeredActivity::SetActivity(pragma::Activity activity) { SetParameter(umath::to_integral(Parameter::Activity), umath::to_integral(activity)); }
void ai::TaskPlayLayeredActivity::SetAnimationSlot(int32_t animationSlot) { SetParameter(umath::to_integral(Parameter::AnimationSlot), animationSlot); }
int32_t ai::TaskPlayLayeredActivity::SelectAnimation(const Schedule *sched, pragma::BaseAIComponent &ent, uint8_t paramId)
{
	auto *param = GetParameter(sched, paramId);
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		return -1;
	auto activity = static_cast<pragma::Activity>(param->GetInt());
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->SelectTranslatedAnimation(activity) : -1;
}
