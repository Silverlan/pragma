// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.liquid.base_buoyancy;

using namespace pragma;

void BaseBuoyancyComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) {}

void BaseBuoyancyComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember) {}
BaseBuoyancyComponent::BaseBuoyancyComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseBuoyancyComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { OnPhysicsInitialized(); });
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PHYSICS_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) { SimulateBuoyancy(); });
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE, [this](std::reference_wrapper<ComponentEvent> evData) { static_cast<CEPostPhysicsSimulate &>(evData.get()).keepAwake = true; });

	auto &ent = GetEntity();
	auto touchC = ent.AddComponent("touch");
	static_cast<BaseTouchComponent *>(touchC.get())->SetNeverDisablePhysicsCallbacks(true);
	ent.AddComponent("model");
	m_surfaceC = ent.AddComponent("surface")->GetHandle<BaseSurfaceComponent>();
	m_liquidControl = ent.AddComponent("liquid_control")->GetHandle<BaseLiquidControlComponent>();
	m_surfSim = ent.AddComponent("liquid_surface_simulation")->GetHandle<BaseLiquidSurfaceSimulationComponent>();
}

void BaseBuoyancyComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	pPhysComponent->SetCollisionFilterMask(physics::CollisionMask::Dynamic | physics::CollisionMask::Generic);
	pPhysComponent->SetCollisionFilterGroup(physics::CollisionMask::Water | physics::CollisionMask::WaterSurface);
	pPhysComponent->SetForcePhysicsAwakeCallbacksEnabled(true);
}

void BaseBuoyancyComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pPhysicsComponent = dynamic_cast<BasePhysicsComponent *>(&component);
	auto *pSurfC = dynamic_cast<BaseSurfaceComponent *>(&component);
	auto *pLiquidControl = dynamic_cast<BaseLiquidControlComponent *>(&component);
	if(pPhysicsComponent != nullptr)
		pPhysicsComponent->SetRayResultCallbackEnabled(true);
	else if(pSurfC)
		m_surfaceC = pSurfC->GetHandle<BaseSurfaceComponent>();
	else if(pLiquidControl)
		m_liquidControl = pLiquidControl->GetHandle<BaseLiquidControlComponent>();
}

util::EventReply BaseBuoyancyComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseTouchComponent::EVENT_CAN_TRIGGER) {
		auto &triggerData = static_cast<CECanTriggerData &>(evData);
		if(triggerData.entity != nullptr) {
			auto pPhysComponent = triggerData.entity->GetPhysicsComponent();
			if(pPhysComponent != nullptr && (pPhysComponent->GetCollisionFilterMask() & physics::CollisionMask::Water) == physics::CollisionMask::None)
				triggerData.canTrigger = false;
		}
		return util::EventReply::Handled;
	}
	return util::EventReply::Unhandled;
}

void BaseBuoyancyComponent::OnEndTouch(ecs::BaseEntity *ent, physics::PhysObj *phys)
{
	auto pSubmergibleComponent = ent->GetComponent<SubmergibleComponent>();
	if(pSubmergibleComponent.valid())
		pSubmergibleComponent->SetSubmergedFraction(GetEntity(), 0.f);
}

void BaseBuoyancyComponent::SimulateBuoyancy() const
{
	if(m_surfaceC.expired() || m_liquidControl.expired())
		return;
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	auto &buoyancySim = physEnv->GetWaterBuoyancySimulator();
	auto *touchComponent = static_cast<BaseTouchComponent *>(ent.FindComponent("touch").get());

	Vector3 n;
	float d;
	m_surfaceC->GetPlaneWs(n, d);

	auto *sim = m_surfSim.valid() ? m_surfSim->GetSurfaceSimulator() : nullptr;
	if(sim != nullptr)
		const_cast<physics::PhysWaterSurfaceSimulator *>(sim)->LockParticleHeights();
	//const std::vector<PhysTouch> &BaseTouchComponent::GetTouchingInfo() const {return m_touching;}
	if(touchComponent != nullptr) {
		for(auto &touchInfo : touchComponent->GetTouchingInfo()) {
			if(touchInfo.touch.entity.valid() == false || touchInfo.triggered == false)
				continue;
			buoyancySim.Simulate(const_cast<ecs::BaseEntity &>(ent), m_liquidControl->GetLiquidDescription(), const_cast<ecs::BaseEntity &>(*touchInfo.touch.entity.get()), n, d, m_liquidControl->GetLiquidVelocity(), sim);
		} // TODO: Trigger has to be higher than max surface height
	}
	if(sim)
		const_cast<physics::PhysWaterSurfaceSimulator *>(sim)->UnlockParticleHeights();
	/*if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->LockParticleHeights();
	auto *ent = m_entity->GetNetworkState()->GetGameState()->FindEntityByClass("prop_physics");
	if(ent != nullptr)
		buoyancySim.Simulate(m_liquid,*ent,n,d,m_waterVelocity,m_physSurfaceSim.get());
	if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->UnlockParticleHeights();*/
}
