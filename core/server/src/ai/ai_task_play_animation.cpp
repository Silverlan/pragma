// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/ai/ai_task_play_animation.h"
#include "pragma/ai/ai_schedule.h"
#include "pragma/entities/components/s_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>

import pragma.server.entities.components;

using namespace pragma;

ai::TaskPlayAnimationBase::TaskPlayAnimationBase(SelectorType selectorType) : ai::BehaviorNode(selectorType), m_resultState(Result::Pending), m_animation(-1) {}
void ai::TaskPlayAnimationBase::Clear()
{
	UnlockAnimation();
	if(m_cbOnPlayAnimation.IsValid())
		m_cbOnPlayAnimation.Remove();
	if(m_cbOnAnimationComplete.IsValid())
		m_cbOnAnimationComplete.Remove();
}
int32_t ai::TaskPlayAnimationBase::SelectAnimation(const Schedule *sched, pragma::SAIComponent &ent, uint8_t paramId)
{
	auto *param = GetParameter(sched, paramId);
	if(param == nullptr)
		return -1;
	auto type = param->GetType();
	int32_t animId = -1;
	switch(type) {
	case ai::Schedule::Parameter::Type::Int:
		{
			animId = param->GetInt();
			break;
		}
	case ai::Schedule::Parameter::Type::String:
		{
			auto &animName = *param->GetString();
			auto mdlComponent = ent.GetEntity().GetModelComponent();
			if(mdlComponent)
				animId = mdlComponent->LookupAnimation(animName);
			break;
		}
	default:
		return -1;
	}
	return animId;
}
void ai::TaskPlayAnimationBase::Print(const Schedule *sched, std::ostream &o) const
{
	o << "PlayAnimation[";
	auto *param = GetParameter(sched, 0);
	if(param == nullptr)
		o << "Nothing";
	else {
		auto type = param->GetType();
		switch(type) {
		case ai::Schedule::Parameter::Type::Int:
			{
				o << param->GetInt();
				break;
			}
		case ai::Schedule::Parameter::Type::String:
			{
				o << *param->GetString();
				break;
			}
		default:
			o << "Nothing";
		}
	}
	o << "]";
}
ai::BehaviorNode::Result ai::TaskPlayAnimationBase::Think(const Schedule *sched, pragma::SAIComponent &ent)
{
	if(m_resultState != Result::Pending)
		return m_resultState;
	if(GetAnimation(ent) == m_animation)
		return Result::Pending;
	return Result::Succeeded;
}

bool ai::TaskPlayAnimationBase::StartTask(const Schedule *sched, pragma::SAIComponent &ent)
{
	BehaviorNode::Start(sched, ent);
	m_resultState = Result::Pending;
	m_animation = SelectAnimation(sched, ent);
	if(m_animation != -1)
		PlayAnimation(ent);
	if(m_animation == -1 || GetAnimation(ent) != m_animation) {
		m_resultState = Result::Failed;
		return false;
	}
	m_targetNpc = &ent;
	ent.LockAnimation(true);
	return true;
}

void ai::TaskPlayAnimationBase::UnlockAnimation()
{
	if(m_targetNpc != nullptr)
		m_targetNpc->LockAnimation(false);
	m_targetNpc = nullptr;
}

void ai::TaskPlayAnimationBase::OnStopped()
{
	BehaviorNode::OnStopped();
	UnlockAnimation();
	Clear();
}

////////////////////////////////////

ai::TaskPlayAnimation::TaskPlayAnimation(SelectorType selectorType) : TaskPlayAnimationBase(selectorType) {}
bool ai::TaskPlayAnimation::GetFaceTarget(const Schedule *sched, pragma::SAIComponent &ent, Vector3 &tgt) const
{
	auto *param = GetParameter(sched, umath::to_integral(Parameter::FaceTarget));
	if(param == nullptr)
		return false;
	auto type = param->GetType();
	if(type == ai::Schedule::Parameter::Type::Vector) {
		tgt = *param->GetVector();
		return true;
	}
	else if(type == ai::Schedule::Parameter::Type::Entity) {
		auto *ent = param->GetEntity();
		if(ent != nullptr) {
			auto pTrComponentEnt = ent->GetTransformComponent();
			if(!pTrComponentEnt)
				return false;
			tgt = pTrComponentEnt->GetPosition();
			return true;
		}
	}
	else if(type == ai::Schedule::Parameter::Type::Bool && param->GetBool() == true) {
		auto *fragment = ent.GetPrimaryTarget();
		if(fragment == nullptr || !fragment->hEntity.valid())
			return false;
		auto pTrComponentTgt = fragment->hEntity.get()->GetTransformComponent();
		if(!pTrComponentTgt)
			return false;
		tgt = pTrComponentTgt->GetPosition();
		return true;
	}
	return false;
}

int32_t ai::TaskPlayAnimation::GetAnimation(pragma::SAIComponent &ent) const
{
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->GetAnimation() : -1;
}

void ai::TaskPlayAnimation::PlayAnimation(pragma::SAIComponent &ent)
{
	ent.StopMoving();

	pragma::SAIComponent::AIAnimationInfo info {};
	info.SetPlayAsSchedule(false);
	ent.PlayAnimation(m_animation, info);
}

void ai::TaskPlayAnimation::SetAnimation(int32_t animation) { SetParameter(umath::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayAnimation::SetAnimation(const std::string &animation) { SetParameter(umath::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayAnimation::SetFaceTarget(const Vector3 &target) { SetParameter(umath::to_integral(Parameter::FaceTarget), target); }
void ai::TaskPlayAnimation::SetFaceTarget(BaseEntity &ent) { SetParameter(umath::to_integral(Parameter::FaceTarget), &ent); }
void ai::TaskPlayAnimation::SetFacePrimaryTarget() { SetParameter(umath::to_integral(Parameter::FaceTarget), true); }

ai::BehaviorNode::Result ai::TaskPlayAnimation::Think(const Schedule *sched, pragma::SAIComponent &ent)
{
	auto r = TaskPlayAnimationBase::Think(sched, ent);
	if(r == Result::Pending) {
		Vector3 faceTarget;
		if(GetFaceTarget(sched, ent, faceTarget) == true)
			ent.TurnStep(faceTarget);
	}
	return r;
}

ai::BehaviorNode::Result ai::TaskPlayAnimation::Start(const Schedule *sched, pragma::SAIComponent &ent)
{
	if(StartTask(sched, ent) == false)
		return Result::Failed;
	auto pAnimComponent = ent.GetEntity().GetAnimatedComponent();
	if(pAnimComponent.expired())
		return Result::Failed;
	m_cbOnAnimationComplete = pAnimComponent->AddEventCallback(pragma::SAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		m_resultState = (static_cast<pragma::CEOnAnimationComplete &>(evData.get()).animation == m_animation) ? Result::Succeeded : Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	m_cbOnPlayAnimation = pAnimComponent->AddEventCallback(pragma::SAnimatedComponent::EVENT_ON_PLAY_ANIMATION, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		m_resultState = Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	return Result::Pending;
}

////////////////////////////////////

ai::TaskPlayLayeredAnimation::TaskPlayLayeredAnimation(ai::SelectorType selectorType) : TaskPlayAnimationBase(selectorType) {}
int32_t ai::TaskPlayLayeredAnimation::GetAnimation(pragma::SAIComponent &ent) const
{
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->GetLayeredAnimation(m_slot) : -1;
}
void ai::TaskPlayLayeredAnimation::PlayAnimation(pragma::SAIComponent &ent)
{
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	if(animComponent.expired())
		return;
	animComponent->PlayLayeredAnimation(m_slot, m_animation);
}
void ai::TaskPlayLayeredAnimation::SetAnimation(int32_t animation) { SetParameter(umath::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayLayeredAnimation::SetAnimation(const std::string &animation) { SetParameter(umath::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayLayeredAnimation::SetAnimationSlot(int32_t animationSlot) { SetParameter(umath::to_integral(Parameter::AnimationSlot), animationSlot); }
ai::BehaviorNode::Result ai::TaskPlayLayeredAnimation::Start(const ai::Schedule *sched, pragma::SAIComponent &ent)
{
	auto *param = GetParameter(sched, umath::to_integral(Parameter::AnimationSlot));
	if(param != nullptr && param->GetType() == ai::Schedule::Parameter::Type::Int)
		m_slot = param->GetInt();

	if(StartTask(sched, ent) == false)
		return Result::Failed;
	auto pAnimComponent = ent.GetEntity().GetAnimatedComponent();
	if(pAnimComponent.expired())
		return Result::Failed;
	m_cbOnAnimationComplete = pAnimComponent->AddEventCallback(pragma::SAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &animInfo = static_cast<pragma::CELayeredAnimationInfo &>(evData.get());
		if(animInfo.slot != m_slot)
			return util::EventReply::Unhandled;
		m_resultState = (animInfo.animation == m_animation) ? Result::Succeeded : Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	m_cbOnPlayAnimation = pAnimComponent->AddEventCallback(pragma::SAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &animInfo = static_cast<pragma::CEOnPlayLayeredAnimation &>(evData.get());
		if(animInfo.slot != m_slot)
			return util::EventReply::Unhandled;
		m_resultState = Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	return Result::Pending;
}
