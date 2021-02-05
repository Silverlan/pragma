/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/func/s_func_water.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/console/s_cvar.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/entities/components/s_model_component.hpp"
#include <pragma/physics/phys_water_surface_simulator.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLSERVER SGame *s_game;

LINK_ENTITY_TO_CLASS(func_water,FuncWater);

static std::vector<SWaterComponent*> s_waterEntities = {};
REGISTER_CONVAR_CALLBACK_CL(sv_water_surface_simulation_shared,[](NetworkState*,ConVar*,int,int val) {
	for(auto *entWater : s_waterEntities)
		entWater->UpdateSurfaceSimulator();
});

SWaterComponent::SWaterComponent(BaseEntity &ent)
	: BaseFuncWaterComponent(ent)
{
	s_waterEntities.push_back(this);
}
SWaterComponent::~SWaterComponent()
{
	auto it = std::find(s_waterEntities.begin(),s_waterEntities.end(),this);
	if(it != s_waterEntities.end())
		s_waterEntities.erase(it);
	if(m_cbClientSimulatorUpdate.IsValid())
		m_cbClientSimulatorUpdate.Remove();
	if(m_cbGameInitialized.IsValid())
		m_cbGameInitialized.Remove();
}

static auto cvSimShared = GetServerConVar("sv_water_surface_simulation_shared");
void SWaterComponent::Initialize()
{
	BaseFuncWaterComponent::Initialize();

	BindEventUnhandled(SModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		// TODO: Move this to shared
		auto &ent = GetEntity();
		if(ent.IsSpawned() == false || m_cbGameInitialized.IsValid() == true)
			return;
		InitializeWaterSurface();
		UpdateSurfaceSimulator();

		/*if(IsShared() == false)
			return;
		NetPacket packet {};
		packet->Write<Vector3>(m_waterPlane.GetNormal());
		packet->Write<double>(m_waterPlane.GetDistance());
		SendNetEventTCP(m_netEvSetWaterPlane,packet);*/
	});
	SetTickPolicy(TickPolicy::Always); // TODO
}

void SWaterComponent::OnTick(double dt)
{
	if(m_bUsingClientsideSimulation == true || m_physSurfaceSim == nullptr)
		return;
	auto *sim = static_cast<PhysWaterSurfaceSimulator*>(m_physSurfaceSim.get());
	if(sim == nullptr)
		return;
	sim->Simulate(0.01); // TODO
}

void SWaterComponent::OnEntitySpawn()
{
	BaseFuncWaterComponent::OnEntitySpawn();
	// TODO: Move this to shared
	auto &ent = GetEntity();
	auto *game = ent.GetNetworkState()->GetGameState();
	if(game->IsMapLoaded() == false)
	{
		// Need to wait until the game is initialized, to be sure the entity exists clientside
		m_cbGameInitialized = game->AddCallback("OnMapLoaded",FunctionCallback<void>::Create([this]() {
			InitializeWaterSurface();
			UpdateSurfaceSimulator();
		}));
		return;
	}
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		return;
	InitializeWaterSurface();
	UpdateSurfaceSimulator();
}

bool SWaterComponent::ShouldSimulateSurface() const {return (BaseFuncWaterComponent::ShouldSimulateSurface() == true && (cvSimShared->GetBool() == true || static_cast<const SBaseEntity&>(GetEntity()).GetClientsideEntity() == nullptr)) ? true : false;}

void SWaterComponent::UpdateSurfaceSimulator()
{
	if(m_cbGameInitialized.IsValid())
		m_cbGameInitialized.Remove();
	m_bUsingClientsideSimulation = false;
	m_physSurfaceSim = nullptr;
	if(cvSimShared->GetBool() == true)
	{
		auto *cent = static_cast<const SBaseEntity&>(GetEntity()).GetClientsideEntity();
		if(cent != nullptr)
		{
			auto *pWaterComponent = static_cast<pragma::BaseFuncWaterComponent*>(cent->FindComponent("water").get());
			if(pWaterComponent != nullptr)
			{
				auto *surfSim = pWaterComponent->GetSurfaceSimulator();
				if(surfSim != nullptr)
				{
					m_bUsingClientsideSimulation = true;
					m_physSurfaceSim = surfSim->shared_from_this();
					auto hEnt = cent->GetHandle();
					m_cbClientSimulatorUpdate = pWaterComponent->BindEventUnhandled(BaseFuncWaterComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED,[this,hEnt](std::reference_wrapper<pragma::ComponentEvent> evData) {
						if(hEnt.IsValid() == false)
							return;
						auto *pWaterComponent = static_cast<pragma::BaseFuncWaterComponent*>(hEnt.get()->FindComponent("water").get());
						auto *surfSim = (pWaterComponent != nullptr) ? pWaterComponent->GetSurfaceSimulator() : nullptr;
						m_physSurfaceSim = (surfSim != nullptr) ? surfSim->shared_from_this() : nullptr;
					});
					return;
				}
			}
		}
	}
	ReloadSurfaceSimulator();
}

void SWaterComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSurfaceMaterial);
	packet->Write<float>(m_kvMaxWaveHeight);
}
void SWaterComponent::CreateSplash(const Vector3 &origin,float radius,float force)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr || pPhysComponent->GetPhysicsObject() == nullptr)
		return;
	BaseFuncWaterComponent::CreateSplash(origin,radius,force);
	if(ent.IsShared() == false)
		return;
	NetPacket packet {};
	packet->Write<Vector3>(origin);
	packet->Write<float>(radius);
	packet->Write<float>(force);
	ent.SendNetEvent(m_netEvCreateSplash,packet,pragma::networking::Protocol::SlowReliable);
}
const Vector3 &SWaterComponent::GetPosition() const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	return pTrComponent != nullptr ? pTrComponent->GetPosition() : uvec::ORIGIN;
}
const Quat &SWaterComponent::GetOrientation() const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	static auto identity = uquat::identity();
	return pTrComponent != nullptr ? pTrComponent->GetRotation() : identity;
}

///////////////

luabind::object SWaterComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SWaterComponentHandleWrapper>(l);}

void FuncWater::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SWaterComponent>();
}
