// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_observer;

using namespace pragma;

void BaseObserverComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseObserverComponent::EVENT_ON_OBSERVATION_MODE_CHANGED = registerEvent("ON_OBSERVATION_MODE_CHANGED", ComponentEventInfo::Type::Broadcast); }

BaseObserverComponent::BaseObserverComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_obsMode(util::TEnumProperty<ObserverMode>::Create(ObserverMode::FirstPerson)) {}

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
	BroadcastEvent(baseObserverComponent::EVENT_ON_OBSERVATION_MODE_CHANGED);
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
	m_observerTarget = pragma::ComponentHandle<BaseObservableComponent> {};
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
