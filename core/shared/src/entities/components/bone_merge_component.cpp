/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/bone_merge_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

bool BoneMergeComponent::can_merge(const Model &mdl, const Model &mdlParent, bool includeRootBones)
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

ComponentEventId BoneMergeComponent::EVENT_ON_TARGET_CHANGED = pragma::INVALID_COMPONENT_ID;
void BoneMergeComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_TARGET_CHANGED = registerEvent("ON_TARGET_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BoneMergeComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BoneMergeComponent;

	{
		using TTarget = pragma::EntityURef;
		auto memberInfo = create_component_member_info<T, TTarget, static_cast<void (T::*)(const TTarget &)>(&T::SetTarget), static_cast<const TTarget &(T::*)() const>(&T::GetTarget)>("target", TTarget {});
		registerMember(std::move(memberInfo));
	}
}
BoneMergeComponent::BoneMergeComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void BoneMergeComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { SetTargetDirty(); });
	BindEventUnhandled(BaseAnimatedComponent::EVENT_POST_ANIMATION_UPDATE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { MergeBonePoses(); });
}
void BoneMergeComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void BoneMergeComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	SetTargetDirty();
}

void BoneMergeComponent::SetTarget(const pragma::EntityURef &target)
{
	m_target = target;
	SetTargetDirty();
	BroadcastEvent(EVENT_ON_TARGET_CHANGED);
}
const pragma::EntityURef &BoneMergeComponent::GetTarget() const { return m_target; }

void BoneMergeComponent::SetTargetDirty()
{
	m_animC = pragma::ComponentHandle<pragma::BaseAnimatedComponent> {};
	m_animCParent = pragma::ComponentHandle<pragma::BaseAnimatedComponent> {};
	m_boneMappings.clear();

	SetTickPolicy(pragma::TickPolicy::Always);
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
		SetTickPolicy(pragma::TickPolicy::Always);
		UpdateBoneMappings();
	}
}
void BoneMergeComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	auto *animC = dynamic_cast<BaseAnimatedComponent *>(&component);
	if(animC) {
		SetTickPolicy(pragma::TickPolicy::Always);
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
		umath::ScaledTransform pose;
		if(!m_animCParent->GetBonePose(mapping.parentBoneId, pose, umath::CoordinateSpace::Object))
			continue;
		m_animC->SetBonePose(mapping.boneId, pose, umath::CoordinateSpace::Object);
	}
	m_animC->SetAbsolutePosesDirty();
}
