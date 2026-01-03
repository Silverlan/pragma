// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ai;

using namespace pragma;

void SAIComponent::AIAnimationInfo::SetPlayFlags(FPlayAnim flags) { m_flags = flags; }
FPlayAnim SAIComponent::AIAnimationInfo::GetPlayFlags() const { return m_flags; }
SAIComponent::AIAnimationInfo::AIAnimFlags SAIComponent::AIAnimationInfo::GetAIAnimFlags() const { return m_aiAnimFlags; }

void SAIComponent::AIAnimationInfo::SetActivity(Activity act) const
{
	math::set_flag(m_aiAnimFlags, AIAnimFlags::PlayAnimation, false);
	math::set_flag(m_aiAnimFlags, AIAnimFlags::PlayActivity, true);
	m_animation.activity = act;
}
void SAIComponent::AIAnimationInfo::SetAnimation(int32_t anim) const
{
	math::set_flag(m_aiAnimFlags, AIAnimFlags::PlayAnimation, true);
	math::set_flag(m_aiAnimFlags, AIAnimFlags::PlayActivity, false);
	m_animation.animation = anim;
}
void SAIComponent::AIAnimationInfo::SetPlayAsSchedule(bool playAsSchedule) { math::set_flag(m_aiAnimFlags, AIAnimFlags::PlayAsSchedule, playAsSchedule); }
bool SAIComponent::AIAnimationInfo::ShouldPlayAsSchedule() const { return math::is_flag_set(m_aiAnimFlags, AIAnimFlags::PlayAsSchedule); }

void SAIComponent::AIAnimationInfo::SetFaceTarget(bool primaryTarget)
{
	math::set_flag(m_aiAnimFlags, AIAnimFlags::FaceEntity | AIAnimFlags::FacePosition, false);
	math::set_flag(m_aiAnimFlags, AIAnimFlags::FacePrimaryTarget, primaryTarget);
	m_faceTarget = nullptr;
}
void SAIComponent::AIAnimationInfo::SetFaceTarget(const Vector3 &position)
{
	math::set_flag(m_aiAnimFlags, AIAnimFlags::FaceEntity | AIAnimFlags::FacePrimaryTarget, false);
	math::set_flag(m_aiAnimFlags, AIAnimFlags::FacePosition, true);
	m_faceTarget = pragma::util::make_shared<Vector3>(position);
}
void SAIComponent::AIAnimationInfo::SetFaceTarget(ecs::BaseEntity &target)
{
	math::set_flag(m_aiAnimFlags, AIAnimFlags::FacePosition | AIAnimFlags::FacePrimaryTarget, false);
	math::set_flag(m_aiAnimFlags, AIAnimFlags::FaceEntity, true);
	m_faceTarget = pragma::util::make_shared<EntityHandle>(target.GetHandle());
}
int32_t SAIComponent::AIAnimationInfo::GetAnimation() const { return m_animation.animation; }
Activity SAIComponent::AIAnimationInfo::GetActivity() const { return m_animation.activity; }
const Vector3 *SAIComponent::AIAnimationInfo::GetFacePosition() const
{
	if(math::is_flag_set(m_aiAnimFlags, AIAnimFlags::FacePosition) == false)
		return nullptr;
	return static_cast<Vector3 *>(m_faceTarget.get());
}
ecs::BaseEntity *SAIComponent::AIAnimationInfo::GetEntityFaceTarget() const
{
	if(math::is_flag_set(m_aiAnimFlags, AIAnimFlags::FaceEntity) == false)
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
	auto bPlayActivity = math::is_flag_set(flags, AIAnimationInfo::AIAnimFlags::PlayActivity);
	if(math::is_flag_set(flags, AIAnimationInfo::AIAnimFlags::PlayAsSchedule)) {
		// TODO: Use info.GetPlayFlags()
		auto sched = ai::Schedule::Create();
		auto taskAnim = bPlayActivity ? pragma::util::make_shared<ai::TaskPlayActivity>() : pragma::util::make_shared<ai::TaskPlayAnimation>();
		taskAnim->SetScheduleParameter(0, 0);
		taskAnim->SetScheduleParameter(1, 1);
		sched->GetRootNode().AddNode(taskAnim);
		if(bPlayActivity == true)
			sched->SetParameter(0, math::to_integral(info.GetActivity()));
		else
			sched->SetParameter(0, info.GetAnimation());
		auto *facePos = info.GetFacePosition();
		auto *faceEnt = info.GetEntityFaceTarget();
		if(facePos != nullptr)
			sched->SetParameter(1, *facePos);
		else if(faceEnt != nullptr)
			sched->SetParameter(1, faceEnt);
		else if(math::is_flag_set(flags, AIAnimationInfo::AIAnimFlags::FacePrimaryTarget)) {
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

void SAIComponent::LockAnimation(bool b) { m_bAnimLocked = b; }
bool SAIComponent::IsAnimationLocked() const { return m_bAnimLocked; }

void SAIComponent::MaintainAnimationMovement(const Vector3 &disp)
{
	if(uvec::length_sqr(disp) == 0.f)
		return;
	Vector3 dirAnimMove = uvec::get_normal(disp);
	Vector3 dirMove = uvec::get_normal(m_posMove); // TODO: Move in actual animation move dir, not disp
	float dot = glm::dot(dirAnimMove, dirMove);
	dot = (dot + 1.f) / 2.f;
	Vector3 mv = disp * dot; // TODO
	                         //SetVelocity(mv);
}
