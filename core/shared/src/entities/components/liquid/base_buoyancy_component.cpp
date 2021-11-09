/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/liquid/base_buoyancy_component.hpp"
#include "pragma/entities/components/liquid/base_liquid_control_component.hpp"
#include "pragma/entities/components/liquid/base_liquid_surface_simulation_component.hpp"
#include "pragma/physics/phys_water_surface_simulator.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_surface_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/phys_water_buoyancy_simulator.hpp"

using namespace pragma;

void BaseBuoyancyComponent::RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent)
{

}

void BaseBuoyancyComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
}
BaseBuoyancyComponent::BaseBuoyancyComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}

void BaseBuoyancyComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnPhysicsInitialized();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		SimulateBuoyancy();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		static_cast<CEPostPhysicsSimulate&>(evData.get()).keepAwake = true;
	});

	auto &ent = GetEntity();
	auto touchC = ent.AddComponent("touch");
	static_cast<BaseTouchComponent*>(touchC.get())->SetNeverDisablePhysicsCallbacks(true);
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
	pPhysComponent->SetCollisionFilterMask(CollisionMask::Dynamic | CollisionMask::Generic);
	pPhysComponent->SetCollisionFilterGroup(CollisionMask::Water | CollisionMask::WaterSurface);
	pPhysComponent->SetForcePhysicsAwakeCallbacksEnabled(true);
}

void BaseBuoyancyComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pPhysicsComponent = dynamic_cast<pragma::BasePhysicsComponent*>(&component);
	auto *pSurfC = dynamic_cast<pragma::BaseSurfaceComponent*>(&component);
	auto *pLiquidControl = dynamic_cast<pragma::BaseLiquidControlComponent*>(&component);
	if(pPhysicsComponent != nullptr)
		pPhysicsComponent->SetRayResultCallbackEnabled(true);
	else if(pSurfC)
		m_surfaceC = pSurfC->GetHandle<BaseSurfaceComponent>();
	else if(pLiquidControl)
		m_liquidControl = pSurfC->GetHandle<BaseLiquidControlComponent>();
}

util::EventReply BaseBuoyancyComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseTouchComponent::EVENT_CAN_TRIGGER)
	{
		auto &triggerData = static_cast<CECanTriggerData&>(evData);
		if(triggerData.entity != nullptr)
		{
			auto pPhysComponent = triggerData.entity->GetPhysicsComponent();
			if(pPhysComponent != nullptr && (pPhysComponent->GetCollisionFilterMask() &CollisionMask::Water) == CollisionMask::None)
				triggerData.canTrigger = false;
		}
		return util::EventReply::Handled;
	}
	return util::EventReply::Unhandled;
}

void BaseBuoyancyComponent::OnEndTouch(BaseEntity *ent,PhysObj *phys)
{
	auto pSubmergibleComponent = ent->GetComponent<pragma::SubmergibleComponent>();
	if(pSubmergibleComponent.valid())
		pSubmergibleComponent->SetSubmergedFraction(GetEntity(),0.f);
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
	auto *touchComponent = static_cast<BaseTouchComponent*>(ent.FindComponent("touch").get());

	Vector3 n;
	float d;
	m_surfaceC->GetPlaneWs(n,d);

	auto *sim = m_surfSim.valid() ? m_surfSim->GetSurfaceSimulator() : nullptr;
	if(sim != nullptr)
		const_cast<PhysWaterSurfaceSimulator*>(sim)->LockParticleHeights();
	//const std::vector<PhysTouch> &BaseTouchComponent::GetTouchingInfo() const {return m_touching;}
	if(touchComponent != nullptr)
	{
		for(auto &touchInfo : touchComponent->GetTouchingInfo())
		{
			if(touchInfo.touch.entity.valid() == false || touchInfo.triggered == false)
				continue;
			buoyancySim.Simulate(
				const_cast<BaseEntity&>(ent),m_liquidControl->GetLiquidDescription(),const_cast<BaseEntity&>(*touchInfo.touch.entity.get()),n,d,
				m_liquidControl->GetLiquidVelocity(),sim
			);
		} // TODO: Trigger has to be higher than max surface height
	}
	if(sim)
		const_cast<PhysWaterSurfaceSimulator*>(sim)->UnlockParticleHeights();
	/*if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->LockParticleHeights();
	auto *ent = m_entity->GetNetworkState()->GetGameState()->FindEntityByClass("prop_physics");
	if(ent != nullptr)
		buoyancySim.Simulate(m_liquid,*ent,n,d,m_waterVelocity,m_physSurfaceSim.get());
	if(m_physSurfaceSim != nullptr)
		m_physSurfaceSim->UnlockParticleHeights();*/
}
