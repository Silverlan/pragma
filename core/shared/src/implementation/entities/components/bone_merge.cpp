// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.bone_merge;

using namespace pragma;

bool BoneMergeComponent::can_merge(const asset::Model &mdl, const asset::Model &mdlParent, bool includeRootBones)
{
	auto &skeleton = mdl.GetSkeleton();
	auto &skeletonParent = mdlParent.GetSkeleton();
	for(auto &bone : skeleton.GetBones()) {
		if(!includeRootBones && bone->parent.expired())
			continue; // We're filtering out root bones because there's usually no point in merging them
		if(skeletonParent.LookupBone(bone->name) != -1)
			return true;
	}
	return false;
}

void BoneMergeComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { boneMergeComponent::EVENT_ON_TARGET_CHANGED = registerEvent("ON_TARGET_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BoneMergeComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BoneMergeComponent;

	{
		using TTarget = EntityURef;
		auto memberInfo = create_component_member_info<T, TTarget, static_cast<void (T::*)(const TTarget &)>(&T::SetTarget), static_cast<const TTarget &(T::*)() const>(&T::GetTarget)>("target", TTarget {});
		registerMember(std::move(memberInfo));
	}
}
BoneMergeComponent::BoneMergeComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void BoneMergeComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { SetTargetDirty(); });
	BindEventUnhandled(baseAnimatedComponent::EVENT_POST_ANIMATION_UPDATE, [this](std::reference_wrapper<ComponentEvent> evData) { MergeBonePoses(); });
}
void BoneMergeComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void BoneMergeComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	SetTargetDirty();
}

void BoneMergeComponent::SetTarget(const EntityURef &target)
{
	m_target = target;
	SetTargetDirty();
	BroadcastEvent(boneMergeComponent::EVENT_ON_TARGET_CHANGED);
}
const EntityURef &BoneMergeComponent::GetTarget() const { return m_target; }

void BoneMergeComponent::SetTargetDirty()
{
	m_animC = pragma::ComponentHandle<BaseAnimatedComponent> {};
	m_animCParent = pragma::ComponentHandle<BaseAnimatedComponent> {};
	m_boneMappings.clear();

	SetTickPolicy(TickPolicy::Always);
	UpdateBoneMappings();
}

void BoneMergeComponent::OnTick(double tDelta)
{
	BaseEntityComponent::OnTick(tDelta);
	UpdateBoneMappings();
}

void BoneMergeComponent::UpdateBoneMappings()
{
	auto &ent = GetEntity();
	auto *entTgt = m_target.GetEntity(GetGame());
	if(!entTgt)
		return;
	auto animC = ent.GetAnimatedComponent();
	auto animCTgt = entTgt->GetAnimatedComponent();
	if(animC.expired() || animCTgt.expired())
		return;
	auto &mdl = ent.GetModel();
	auto &mdlTgt = entTgt->GetModel();
	if(!mdl || !mdlTgt)
		return;
	auto &skeleton = mdl->GetSkeleton();
	auto &skeletonTgt = mdlTgt->GetSkeleton();
	for(auto &boneTgt : skeletonTgt.GetBones()) {
		auto boneId = skeleton.LookupBone(boneTgt->name);
		if(boneId == -1)
			continue;
		if(m_boneMappings.size() == m_boneMappings.capacity())
			m_boneMappings.reserve(m_boneMappings.size() * 2 + 10);
		m_boneMappings.push_back({});
		auto &mapping = m_boneMappings.back();
		mapping.boneId = boneId;
		mapping.parentBoneId = boneTgt->ID;
	}
	m_animC = animC;
	m_animCParent = animCTgt;
	animC->SetPostAnimationUpdateEnabled(true);
	SetTickPolicy(TickPolicy::Never);
}
void BoneMergeComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *animC = dynamic_cast<BaseAnimatedComponent *>(&component);
	if(animC) {
		SetTickPolicy(TickPolicy::Always);
		UpdateBoneMappings();
	}
}
void BoneMergeComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	auto *animC = dynamic_cast<BaseAnimatedComponent *>(&component);
	if(animC) {
		SetTickPolicy(TickPolicy::Always);
		UpdateBoneMappings();
	}
}
void BoneMergeComponent::MergeBonePoses()
{
	if(m_animC.expired() || m_animCParent.expired())
		return;
	auto &poses = m_animC->GetBoneTransforms();
	auto &posesParent = m_animCParent->GetBoneTransforms();
	for(auto &mapping : m_boneMappings) {
		if(mapping.boneId >= poses.size() || mapping.parentBoneId >= posesParent.size())
			continue;
		math::ScaledTransform pose;
		if(!m_animCParent->GetBonePose(mapping.parentBoneId, pose, math::CoordinateSpace::Object))
			continue;
		m_animC->SetBonePose(mapping.boneId, pose, math::CoordinateSpace::Object);
	}
	m_animC->SetAbsolutePosesDirty();
}
