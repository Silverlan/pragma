/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include <pragma/engine.h>
#include "pragma/entities/components/base_observer_component.hpp"
#include "pragma/entities/components/base_observable_component.hpp"

using namespace pragma;

ComponentEventId BaseObserverComponent::EVENT_ON_OBSERVATION_MODE_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseObserverComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_OBSERVATION_MODE_CHANGED = registerEvent("ON_OBSERVATION_MODE_CHANGED", ComponentEventInfo::Type::Broadcast); }

BaseObserverComponent::BaseObserverComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_obsMode(util::TEnumProperty<ObserverMode>::Create(ObserverMode::FirstPerson)) {}

BaseObserverComponent::~BaseObserverComponent() {}

void BaseObserverComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvSetObserverMode = SetupNetEvent("set_observer_mode");
	m_netEvSetObserverTarget = SetupNetEvent("set_observer_target");

	auto &ent = GetEntity();
	auto whObservableComponent = ent.FindComponent("observable");
	if(whObservableComponent.valid()) {
		auto *pObservableComponent = static_cast<BaseObservableComponent *>(whObservableComponent.get());
		pObservableComponent->SetCameraEnabled(BaseObservableComponent::CameraType::FirstPerson, true);
		pObservableComponent->SetCameraEnabled(BaseObservableComponent::CameraType::ThirdPerson, true);
		pObservableComponent->SetLocalCameraOffset(BaseObservableComponent::CameraType::ThirdPerson, {0.f, 10.f, -80.f});
	}
}

void BaseObserverComponent::SetObserverMode(ObserverMode mode)
{
	*m_obsMode = mode;
	DoSetObserverMode(mode);
	BroadcastEvent(EVENT_ON_OBSERVATION_MODE_CHANGED);
}
ObserverMode BaseObserverComponent::GetObserverMode() const { return *m_obsMode; }
const util::PEnumProperty<ObserverMode> &BaseObserverComponent::GetObserverModeProperty() const { return m_obsMode; }
void BaseObserverComponent::ClearObserverTarget()
{
	if(m_observerTarget.expired())
		return;
	m_observerTarget->SetObserver(nullptr);
	m_observerTarget = pragma::ComponentHandle<BaseObservableComponent> {};
}
void BaseObserverComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	ClearObserverTarget();
}
void BaseObserverComponent::SetObserverTarget(BaseObservableComponent *ent)
{
	ClearObserverTarget();
	m_observerTarget = pragma::ComponentHandle<pragma::BaseObservableComponent> {};
	if(ent == nullptr)
		return;
	m_observerTarget = ent->GetHandle<BaseObservableComponent>();
	m_observerTarget->SetObserver(this);
}
BaseObservableComponent *BaseObserverComponent::GetObserverTarget() const
{
	auto *r = const_cast<BaseObservableComponent *>(m_observerTarget.get());
	if(r == nullptr) {
		auto pObsComponent = GetEntity().FindComponent("observable");
		return static_cast<BaseObservableComponent *>(pObsComponent.get());
	}
	return r;
}
