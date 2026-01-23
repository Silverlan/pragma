// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :ai.tasks.play_animation;

import :entities.components;

using namespace pragma;

ai::TaskPlayAnimationBase::TaskPlayAnimationBase(SelectorType selectorType) : BehaviorNode(selectorType), m_resultState(Result::Pending), m_animation(-1) {}
void ai::TaskPlayAnimationBase::Clear()
{
	UnlockAnimation();
	if(m_cbOnPlayAnimation.IsValid())
		m_cbOnPlayAnimation.Remove();
	if(m_cbOnAnimationComplete.IsValid())
		m_cbOnAnimationComplete.Remove();
}
int32_t ai::TaskPlayAnimationBase::SelectAnimation(const Schedule *sched, BaseAIComponent &ent, uint8_t paramId)
{
	auto *param = GetParameter(sched, paramId);
	if(param == nullptr)
		return -1;
	auto type = param->GetType();
	int32_t animId = -1;
	switch(type) {
	case Schedule::Parameter::Type::Int:
		{
			animId = param->GetInt();
			break;
		}
	case Schedule::Parameter::Type::String:
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
		case Schedule::Parameter::Type::Int:
			{
				o << param->GetInt();
				break;
			}
		case Schedule::Parameter::Type::String:
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
ai::BehaviorNode::Result ai::TaskPlayAnimationBase::Think(const Schedule *sched, BaseAIComponent &ent)
{
	if(m_resultState != Result::Pending)
		return m_resultState;
	if(GetAnimation(ent) == m_animation)
		return Result::Pending;
	return Result::Succeeded;
}

bool ai::TaskPlayAnimationBase::StartTask(const Schedule *sched, BaseAIComponent &ent)
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
	static_cast<SAIComponent &>(ent).LockAnimation(true);
	return true;
}

void ai::TaskPlayAnimationBase::UnlockAnimation()
{
	if(m_targetNpc != nullptr)
		static_cast<SAIComponent &>(*m_targetNpc).LockAnimation(false);
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
bool ai::TaskPlayAnimation::GetFaceTarget(const Schedule *sched, BaseAIComponent &ent, Vector3 &tgt) const
{
	auto *param = GetParameter(sched, math::to_integral(Parameter::FaceTarget));
	if(param == nullptr)
		return false;
	auto type = param->GetType();
	if(type == Schedule::Parameter::Type::Vector) {
		tgt = *param->GetVector();
		return true;
	}
	else if(type == Schedule::Parameter::Type::Entity) {
		auto *ent = param->GetEntity();
		if(ent != nullptr) {
			auto pTrComponentEnt = ent->GetTransformComponent();
			if(!pTrComponentEnt)
				return false;
			tgt = pTrComponentEnt->GetPosition();
			return true;
		}
	}
	else if(type == Schedule::Parameter::Type::Bool && param->GetBool() == true) {
		auto *fragment = static_cast<SAIComponent &>(ent).GetPrimaryTarget();
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

int32_t ai::TaskPlayAnimation::GetAnimation(BaseAIComponent &ent) const
{
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->GetAnimation() : -1;
}

void ai::TaskPlayAnimation::PlayAnimation(BaseAIComponent &ent)
{
	ent.StopMoving();

	SAIComponent::AIAnimationInfo info {};
	info.SetPlayAsSchedule(false);
	static_cast<SAIComponent &>(ent).PlayAnimation(m_animation, info);
}

void ai::TaskPlayAnimation::SetAnimation(int32_t animation) { SetParameter(math::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayAnimation::SetAnimation(const std::string &animation) { SetParameter(math::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayAnimation::SetFaceTarget(const Vector3 &target) { SetParameter(math::to_integral(Parameter::FaceTarget), target); }
void ai::TaskPlayAnimation::SetFaceTarget(ecs::BaseEntity &ent) { SetParameter(math::to_integral(Parameter::FaceTarget), &ent); }
void ai::TaskPlayAnimation::SetFacePrimaryTarget() { SetParameter(math::to_integral(Parameter::FaceTarget), true); }

ai::BehaviorNode::Result ai::TaskPlayAnimation::Think(const Schedule *sched, BaseAIComponent &ent)
{
	auto r = TaskPlayAnimationBase::Think(sched, ent);
	if(r == Result::Pending) {
		Vector3 faceTarget;
		if(GetFaceTarget(sched, ent, faceTarget) == true)
			ent.TurnStep(faceTarget);
	}
	return r;
}

ai::BehaviorNode::Result ai::TaskPlayAnimation::Start(const Schedule *sched, BaseAIComponent &ent)
{
	if(StartTask(sched, ent) == false)
		return Result::Failed;
	auto pAnimComponent = ent.GetEntity().GetAnimatedComponent();
	if(pAnimComponent.expired())
		return Result::Failed;
	m_cbOnAnimationComplete = pAnimComponent->AddEventCallback(sAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		m_resultState = (static_cast<CEOnAnimationComplete &>(evData.get()).animation == m_animation) ? Result::Succeeded : Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	m_cbOnPlayAnimation = pAnimComponent->AddEventCallback(sAnimatedComponent::EVENT_ON_PLAY_ANIMATION, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		m_resultState = Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	return Result::Pending;
}

////////////////////////////////////

ai::TaskPlayLayeredAnimation::TaskPlayLayeredAnimation(SelectorType selectorType) : TaskPlayAnimationBase(selectorType) {}
int32_t ai::TaskPlayLayeredAnimation::GetAnimation(BaseAIComponent &ent) const
{
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	return animComponent.valid() ? animComponent->GetLayeredAnimation(m_slot) : -1;
}
void ai::TaskPlayLayeredAnimation::PlayAnimation(BaseAIComponent &ent)
{
	auto animComponent = ent.GetEntity().GetAnimatedComponent();
	if(animComponent.expired())
		return;
	animComponent->PlayLayeredAnimation(m_slot, m_animation);
}
void ai::TaskPlayLayeredAnimation::SetAnimation(int32_t animation) { SetParameter(math::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayLayeredAnimation::SetAnimation(const std::string &animation) { SetParameter(math::to_integral(Parameter::Animation), animation); }
void ai::TaskPlayLayeredAnimation::SetAnimationSlot(int32_t animationSlot) { SetParameter(math::to_integral(Parameter::AnimationSlot), animationSlot); }
ai::BehaviorNode::Result ai::TaskPlayLayeredAnimation::Start(const Schedule *sched, BaseAIComponent &ent)
{
	auto *param = GetParameter(sched, math::to_integral(Parameter::AnimationSlot));
	if(param != nullptr && param->GetType() == Schedule::Parameter::Type::Int)
		m_slot = param->GetInt();

	if(StartTask(sched, ent) == false)
		return Result::Failed;
	auto pAnimComponent = ent.GetEntity().GetAnimatedComponent();
	if(pAnimComponent.expired())
		return Result::Failed;
	m_cbOnAnimationComplete = pAnimComponent->AddEventCallback(sAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &animInfo = static_cast<CELayeredAnimationInfo &>(evData.get());
		if(animInfo.slot != m_slot)
			return util::EventReply::Unhandled;
		m_resultState = (animInfo.animation == m_animation) ? Result::Succeeded : Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	m_cbOnPlayAnimation = pAnimComponent->AddEventCallback(sAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &animInfo = static_cast<CEOnPlayLayeredAnimation &>(evData.get());
		if(animInfo.slot != m_slot)
			return util::EventReply::Unhandled;
		m_resultState = Result::Failed;
		Clear();
		return util::EventReply::Unhandled;
	});
	return Result::Pending;
}
