/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/liquid/s_liquid_component.hpp"
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
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLSERVER SGame *s_game;

LINK_ENTITY_TO_CLASS(func_water,FuncWater);

static std::vector<SLiquidComponent*> s_waterEntities = {};
REGISTER_CONVAR_CALLBACK_CL(sv_water_surface_simulation_shared,[](NetworkState*,ConVar*,int,int val) {
	for(auto *entWater : s_waterEntities)
		entWater->UpdateSurfaceSimulator();
});

SLiquidComponent::SLiquidComponent(BaseEntity &ent)
	: BaseFuncLiquidComponent(ent)
{
	s_waterEntities.push_back(this);
}
SLiquidComponent::~SLiquidComponent()
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
void SLiquidComponent::Initialize()
{
	BaseFuncLiquidComponent::Initialize();

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

void SLiquidComponent::OnTick(double dt)
{
	if(m_bUsingClientsideSimulation == true || m_physSurfaceSim == nullptr)
		return;
	auto *sim = static_cast<PhysWaterSurfaceSimulator*>(m_physSurfaceSim.get());
	if(sim == nullptr)
		return;
	sim->Simulate(0.01); // TODO
}

void SLiquidComponent::OnEntitySpawn()
{
	BaseFuncLiquidComponent::OnEntitySpawn();
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

bool SLiquidComponent::ShouldSimulateSurface() const {return (BaseFuncLiquidComponent::ShouldSimulateSurface() == true && (cvSimShared->GetBool() == true || static_cast<const SBaseEntity&>(GetEntity()).GetClientsideEntity() == nullptr)) ? true : false;}

void SLiquidComponent::UpdateSurfaceSimulator()
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
			auto *pWaterComponent = static_cast<pragma::BaseFuncLiquidComponent*>(cent->FindComponent("water").get());
			if(pWaterComponent != nullptr)
			{
				auto *surfSim = pWaterComponent->GetSurfaceSimulator();
				if(surfSim != nullptr)
				{
					m_bUsingClientsideSimulation = true;
					m_physSurfaceSim = surfSim->shared_from_this();
					auto hEnt = cent->GetHandle();
					m_cbClientSimulatorUpdate = pWaterComponent->BindEventUnhandled(BaseFuncLiquidComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED,[this,hEnt](std::reference_wrapper<pragma::ComponentEvent> evData) {
						if(hEnt.valid() == false)
							return;
						auto *pWaterComponent = static_cast<pragma::BaseFuncLiquidComponent*>(hEnt.get()->FindComponent("water").get());
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

void SLiquidComponent::SendData(NetPacket &packet,networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_kvSurfaceMaterial);
	packet->Write<float>(m_kvMaxWaveHeight);
}
void SLiquidComponent::CreateSplash(const Vector3 &origin,float radius,float force)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent == nullptr || pPhysComponent->GetPhysicsObject() == nullptr)
		return;
	BaseFuncLiquidComponent::CreateSplash(origin,radius,force);
	if(ent.IsShared() == false)
		return;
	NetPacket packet {};
	packet->Write<Vector3>(origin);
	packet->Write<float>(radius);
	packet->Write<float>(force);
	ent.SendNetEvent(m_netEvCreateSplash,packet,pragma::networking::Protocol::SlowReliable);
}

///////////////

void SLiquidComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

void FuncWater::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLiquidComponent>();
}
