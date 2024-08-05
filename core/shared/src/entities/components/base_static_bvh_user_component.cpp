/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_static_bvh_user_component.hpp"
#include "pragma/entities/components/base_static_bvh_cache_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"

using namespace pragma;

ComponentEventId BaseStaticBvhUserComponent::EVENT_ON_ACTIVATION_STATE_CHANGED = INVALID_COMPONENT_ID;
ComponentEventId BaseStaticBvhUserComponent::EVENT_ON_STATIC_BVH_COMPONENT_CHANGED = INVALID_COMPONENT_ID;
void BaseStaticBvhUserComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_ACTIVATION_STATE_CHANGED = registerEvent("ON_ACTIVATION_STATE_CHANGED", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_STATIC_BVH_COMPONENT_CHANGED = registerEvent("ON_STATIC_BVH_COMPONENT_CHANGED", ComponentEventInfo::Type::Broadcast);
}

BaseStaticBvhUserComponent::BaseStaticBvhUserComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
BaseStaticBvhUserComponent::~BaseStaticBvhUserComponent() {}
void BaseStaticBvhUserComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr) {
		auto &trC = *pTrComponent;
		if(m_cbOnPoseChanged.IsValid())
			m_cbOnPoseChanged.Remove();
		m_cbOnPoseChanged = pTrComponent->AddEventCallback(BaseTransformComponent::EVENT_ON_POSE_CHANGED, [this, &trC](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			if(m_staticBvhComponent.valid())
				m_staticBvhComponent->SetEntityDirty(GetEntity());
			return util::EventReply::Unhandled;
		});
	}
}
void BaseStaticBvhUserComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	UpdateBvhStatus();
}
void BaseStaticBvhUserComponent::UpdateBvhStatus()
{
	if(HasDynamicBvhSubstitute()) {
		if(m_isActive) {
			m_isActive = false;
			BroadcastEvent(EVENT_ON_ACTIVATION_STATE_CHANGED);
		}
		return;
	}

	auto isStatic = GetEntity().IsStatic();
	if(m_panimaComponent)
		isStatic = false;

	if(m_staticBvhComponent.expired()) {
		if(m_isActive) {
			m_isActive = false;
			BroadcastEvent(EVENT_ON_ACTIVATION_STATE_CHANGED);
		}
		return;
	}
	if(isStatic) {
		m_staticBvhComponent->AddEntity(GetEntity());
		if(!m_isActive) {
			m_isActive = true;
			BroadcastEvent(EVENT_ON_ACTIVATION_STATE_CHANGED);
		}
	}
	else {
		m_staticBvhComponent->RemoveEntity(GetEntity(), false);
		if(m_isActive) {
			m_isActive = false;
			BroadcastEvent(EVENT_ON_ACTIVATION_STATE_CHANGED);
		}
	}
}
void BaseStaticBvhUserComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(PanimaComponent)) {
		m_panimaComponent = static_cast<PanimaComponent *>(&component);
		UpdateBvhStatus();
	}
}
void BaseStaticBvhUserComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(PanimaComponent)) {
		m_panimaComponent = nullptr;
		UpdateBvhStatus();
	}
}
bool BaseStaticBvhUserComponent::IsActive() const { return m_isActive; }
util::EventReply BaseStaticBvhUserComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(eventId == BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED || eventId == BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED)
		UpdateBvhStatus();
	return BaseEntityComponent::HandleEvent(eventId, evData);
}
void BaseStaticBvhUserComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_cbOnPoseChanged.IsValid())
		m_cbOnPoseChanged.Remove();
	if(m_staticBvhComponent.valid())
		m_staticBvhComponent->RemoveEntity(GetEntity());
}
void BaseStaticBvhUserComponent::SetStaticBvhCacheComponent(BaseStaticBvhCacheComponent *component)
{
	m_staticBvhComponent = component ? component->GetHandle<BaseStaticBvhCacheComponent>() : pragma::ComponentHandle<BaseStaticBvhCacheComponent> {};
	BroadcastEvent(EVENT_ON_STATIC_BVH_COMPONENT_CHANGED);
}
void BaseStaticBvhUserComponent::InitializeDynamicBvhSubstitute(size_t staticBvhCacheVersion)
{
	m_staticBvhCacheVersion = staticBvhCacheVersion;
	GetEntity().AddComponent("bvh");
	UpdateBvhStatus();
}
bool BaseStaticBvhUserComponent::HasDynamicBvhSubstitute() const { return GetEntity().FindComponent("bvh").valid(); }
void BaseStaticBvhUserComponent::DestroyDynamicBvhSubstitute()
{
	GetEntity().RemoveComponent("bvh");
	UpdateBvhStatus();
}
