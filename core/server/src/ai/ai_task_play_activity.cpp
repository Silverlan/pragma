/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/ai/ai_task_play_activity.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_animated_component.hpp"

using namespace pragma;

int32_t ai::TaskPlayActivity::SelectAnimation(const Schedule *sched,pragma::SAIComponent &ent,uint8_t paramId)
{
	auto *param = GetParameter(sched,paramId);
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		return -1;
	auto activity = static_cast<Activity>(param->GetInt());
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->SelectTranslatedAnimation(activity) : -1;
}

void ai::TaskPlayActivity::SetActivity(Activity activity)
{
	SetParameter(umath::to_integral(Parameter::Activity),umath::to_integral(activity));
}
void ai::TaskPlayActivity::SetFaceTarget(const Vector3 &target)
{
	SetParameter(umath::to_integral(Parameter::FaceTarget),target);
}
void ai::TaskPlayActivity::SetFaceTarget(BaseEntity &target)
{
	SetParameter(umath::to_integral(Parameter::FaceTarget),&target);
}

void ai::TaskPlayActivity::Print(const Schedule *sched,std::ostream &o) const
{
	o<<"PlayActivity[";
	auto *param = GetParameter(sched,umath::to_integral(Parameter::Activity));
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		o<<"Nothing";
	else
	{
		auto actId = param->GetInt();
		auto it = ACTIVITY_NAMES.find(actId);
		if(it != ACTIVITY_NAMES.end())
			o<<it->second;
		else
			o<<actId;
	}
	o<<"]";
}

////////////////////////////////////

void ai::TaskPlayLayeredActivity::Print(const Schedule *sched,std::ostream &o) const
{
	auto *pSlot = GetParameter(umath::to_integral(Parameter::AnimationSlot));
	o<<"PlayLayeredActivity[";
	auto *param = GetParameter(sched,umath::to_integral(Parameter::Activity));
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		o<<"Nothing";
	else
	{
		auto actId = param->GetInt();
		auto it = ACTIVITY_NAMES.find(actId);
		if(it != ACTIVITY_NAMES.end())
			o<<it->second;
		else
			o<<actId;
	}
	o<<"][";
	if(pSlot != nullptr && pSlot->GetType() == pragma::ai::BehaviorParameter::Type::Int)
		o<<pSlot->GetInt();
	else
		o<<"NULL";
	o<<"]";
}
void ai::TaskPlayLayeredActivity::SetActivity(Activity activity)
{
	SetParameter(umath::to_integral(Parameter::Activity),umath::to_integral(activity));
}
void ai::TaskPlayLayeredActivity::SetAnimationSlot(int32_t animationSlot)
{
	SetParameter(umath::to_integral(Parameter::AnimationSlot),animationSlot);
}
int32_t ai::TaskPlayLayeredActivity::SelectAnimation(const Schedule *sched,pragma::SAIComponent &ent,uint8_t paramId)
{
	auto *param = GetParameter(sched,paramId);
	if(param == nullptr || param->GetType() != ai::Schedule::Parameter::Type::Int)
		return -1;
	auto activity = static_cast<Activity>(param->GetInt());
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->SelectTranslatedAnimation(activity) : -1;
}
