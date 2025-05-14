/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_animated_component.hpp"
#include "pragma/ai/ai_schedule.h"
#include "pragma/ai/ai_task_play_activity.h"

using namespace pragma;

void SAIComponent::AIAnimationInfo::SetPlayFlags(pragma::FPlayAnim flags) { m_flags = flags; }
pragma::FPlayAnim SAIComponent::AIAnimationInfo::GetPlayFlags() const { return m_flags; }
SAIComponent::AIAnimationInfo::AIAnimFlags SAIComponent::AIAnimationInfo::GetAIAnimFlags() const { return m_aiAnimFlags; }

void SAIComponent::AIAnimationInfo::SetActivity(Activity act) const
{
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::PlayAnimation, false);
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::PlayActivity, true);
	m_animation.activity = act;
}
void SAIComponent::AIAnimationInfo::SetAnimation(int32_t anim) const
{
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::PlayAnimation, true);
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::PlayActivity, false);
	m_animation.animation = anim;
}
void SAIComponent::AIAnimationInfo::SetPlayAsSchedule(bool playAsSchedule) { umath::set_flag(m_aiAnimFlags, AIAnimFlags::PlayAsSchedule, playAsSchedule); }
bool SAIComponent::AIAnimationInfo::ShouldPlayAsSchedule() const { return umath::is_flag_set(m_aiAnimFlags, AIAnimFlags::PlayAsSchedule); }

void SAIComponent::AIAnimationInfo::SetFaceTarget(bool primaryTarget)
{
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::FaceEntity | AIAnimFlags::FacePosition, false);
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::FacePrimaryTarget, primaryTarget);
	m_faceTarget = nullptr;
}
void SAIComponent::AIAnimationInfo::SetFaceTarget(const Vector3 &position)
{
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::FaceEntity | AIAnimFlags::FacePrimaryTarget, false);
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::FacePosition, true);
	m_faceTarget = std::make_shared<Vector3>(position);
}
void SAIComponent::AIAnimationInfo::SetFaceTarget(BaseEntity &target)
{
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::FacePosition | AIAnimFlags::FacePrimaryTarget, false);
	umath::set_flag(m_aiAnimFlags, AIAnimFlags::FaceEntity, true);
	m_faceTarget = std::make_shared<EntityHandle>(target.GetHandle());
}
int32_t SAIComponent::AIAnimationInfo::GetAnimation() const { return m_animation.animation; }
Activity SAIComponent::AIAnimationInfo::GetActivity() const { return m_animation.activity; }
const Vector3 *SAIComponent::AIAnimationInfo::GetFacePosition() const
{
	if(umath::is_flag_set(m_aiAnimFlags, AIAnimFlags::FacePosition) == false)
		return nullptr;
	return static_cast<Vector3 *>(m_faceTarget.get());
}
BaseEntity *SAIComponent::AIAnimationInfo::GetEntityFaceTarget() const
{
	if(umath::is_flag_set(m_aiAnimFlags, AIAnimFlags::FaceEntity) == false)
		return nullptr;
	auto *hEnt = static_cast<EntityHandle *>(m_faceTarget.get());
	return (hEnt != nullptr && hEnt->valid()) ? hEnt->get() : nullptr;
}

//////////////////////

bool SAIComponent::PlayAnimation(const AIAnimationInfo &info)
{
	if(IsAnimationLocked())
		return false;
	auto flags = info.GetAIAnimFlags();
	auto bPlayActivity = umath::is_flag_set(flags, AIAnimationInfo::AIAnimFlags::PlayActivity);
	if(umath::is_flag_set(flags, AIAnimationInfo::AIAnimFlags::PlayAsSchedule)) {
		// TODO: Use info.GetPlayFlags()
		auto sched = ai::Schedule::Create();
		auto taskAnim = bPlayActivity ? std::make_shared<ai::TaskPlayActivity>() : std::make_shared<ai::TaskPlayAnimation>();
		taskAnim->SetScheduleParameter(0, 0);
		taskAnim->SetScheduleParameter(1, 1);
		sched->GetRootNode().AddNode(taskAnim);
		if(bPlayActivity == true)
			sched->SetParameter(0, umath::to_integral(info.GetActivity()));
		else
			sched->SetParameter(0, info.GetAnimation());
		auto *facePos = info.GetFacePosition();
		auto *faceEnt = info.GetEntityFaceTarget();
		if(facePos != nullptr)
			sched->SetParameter(1, *facePos);
		else if(faceEnt != nullptr)
			sched->SetParameter(1, faceEnt);
		else if(umath::is_flag_set(flags, AIAnimationInfo::AIAnimFlags::FacePrimaryTarget)) {
			auto *pFragment = GetPrimaryTarget();
			if(pFragment != nullptr && pFragment->hEntity.valid())
				sched->SetParameter(1, pFragment->hEntity.get());
		}
		m_bSkipHandling = true;
		StartSchedule(sched);
		m_bSkipHandling = false;
		return true;
	}
	auto animComponent = GetEntity().GetAnimatedComponent();
	if(animComponent.expired())
		return false;
	m_bSkipHandling = true;
	auto r = true;
	if(bPlayActivity)
		r = animComponent->PlayActivity(info.GetActivity(), info.GetPlayFlags());
	else
		animComponent->PlayAnimation(info.GetAnimation(), info.GetPlayFlags());
	m_bSkipHandling = false;
	return r;
}
bool SAIComponent::PlayActivity(Activity act, const AIAnimationInfo &info)
{
	info.SetActivity(act);
	return PlayAnimation(info);
}
bool SAIComponent::PlayAnimation(int32_t anim, const AIAnimationInfo &info)
{
	info.SetAnimation(anim);
	return PlayAnimation(info);
}
