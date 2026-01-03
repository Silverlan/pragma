// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.play_activity;

import :entities.components;

using namespace pragma;

int32_t ai::TaskPlayActivity::SelectAnimation(const Schedule *sched, BaseAIComponent &ent, uint8_t paramId)
{
	auto *param = GetParameter(sched, paramId);
	if(param == nullptr || param->GetType() != Schedule::Parameter::Type::Int)
		return -1;
	auto activity = static_cast<Activity>(param->GetInt());
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->SelectTranslatedAnimation(activity) : -1;
}

void ai::TaskPlayActivity::SetActivity(Activity activity) { SetParameter(math::to_integral(Parameter::Activity), math::to_integral(activity)); }
void ai::TaskPlayActivity::SetFaceTarget(const Vector3 &target) { SetParameter(math::to_integral(Parameter::FaceTarget), target); }
void ai::TaskPlayActivity::SetFaceTarget(ecs::BaseEntity &target) { SetParameter(math::to_integral(Parameter::FaceTarget), &target); }

void ai::TaskPlayActivity::Print(const Schedule *sched, std::ostream &o) const
{
	o << "PlayActivity[";
	auto *param = GetParameter(sched, math::to_integral(Parameter::Activity));
	if(param == nullptr || param->GetType() != Schedule::Parameter::Type::Int)
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
	auto *pSlot = GetParameter(math::to_integral(Parameter::AnimationSlot));
	o << "PlayLayeredActivity[";
	auto *param = GetParameter(sched, math::to_integral(Parameter::Activity));
	if(param == nullptr || param->GetType() != Schedule::Parameter::Type::Int)
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
	if(pSlot != nullptr && pSlot->GetType() == BehaviorParameter::Type::Int)
		o << pSlot->GetInt();
	else
		o << "NULL";
	o << "]";
}
void ai::TaskPlayLayeredActivity::SetActivity(Activity activity) { SetParameter(math::to_integral(Parameter::Activity), math::to_integral(activity)); }
void ai::TaskPlayLayeredActivity::SetAnimationSlot(int32_t animationSlot) { SetParameter(math::to_integral(Parameter::AnimationSlot), animationSlot); }
int32_t ai::TaskPlayLayeredActivity::SelectAnimation(const Schedule *sched, BaseAIComponent &ent, uint8_t paramId)
{
	auto *param = GetParameter(sched, paramId);
	if(param == nullptr || param->GetType() != Schedule::Parameter::Type::Int)
		return -1;
	auto activity = static_cast<Activity>(param->GetInt());
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->SelectTranslatedAnimation(activity) : -1;
}
