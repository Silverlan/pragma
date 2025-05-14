/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/liquid/s_liquid_surface_simulation_component.hpp"
#include "pragma/console/s_cvar.h"
#include <pragma/physics/phys_water_surface_simulator.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_surface_component.hpp>
#include <sharedutils/netpacket.hpp>

using namespace pragma;

static std::vector<SLiquidSurfaceSimulationComponent *> s_waterEntities = {};
REGISTER_CONVAR_CALLBACK_CL(sv_water_surface_simulation_shared, [](NetworkState *, const ConVar &, int, int val) {
	for(auto *entWater : s_waterEntities)
		entWater->UpdateSurfaceSimulator();
});

SLiquidSurfaceSimulationComponent::SLiquidSurfaceSimulationComponent(BaseEntity &ent) : BaseLiquidSurfaceSimulationComponent(ent) { s_waterEntities.push_back(this); }
SLiquidSurfaceSimulationComponent::~SLiquidSurfaceSimulationComponent()
{
	if(m_cbClientSimulatorUpdate.IsValid())
		m_cbClientSimulatorUpdate.Remove();

	auto it = std::find(s_waterEntities.begin(), s_waterEntities.end(), this);
	if(it != s_waterEntities.end())
		s_waterEntities.erase(it);
}

void SLiquidSurfaceSimulationComponent::Initialize()
{
	BaseLiquidSurfaceSimulationComponent::Initialize();
	BindEventUnhandled(BaseSurfaceComponent::EVENT_ON_SURFACE_MESH_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { UpdateSurfaceSimulator(); });
}

void SLiquidSurfaceSimulationComponent::InitializeLuaObject(lua_State *l) { return BaseLiquidSurfaceSimulationComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidSurfaceSimulationComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<float>(m_kvMaxWaveHeight); }

static auto cvSimShared = GetServerConVar("sv_water_surface_simulation_shared");
bool SLiquidSurfaceSimulationComponent::ShouldSimulateSurface() const { return (BaseLiquidSurfaceSimulationComponent::ShouldSimulateSurface() == true && (cvSimShared->GetBool() == true || static_cast<const SBaseEntity &>(GetEntity()).GetClientsideEntity() == nullptr)) ? true : false; }

void SLiquidSurfaceSimulationComponent::OnTick(double dt)
{
	if(m_bUsingClientsideSimulation == true || m_physSurfaceSim == nullptr)
		return;
	auto *sim = static_cast<PhysWaterSurfaceSimulator *>(m_physSurfaceSim.get());
	if(sim == nullptr)
		return;
	sim->Simulate(0.01); // TODO
}

void SLiquidSurfaceSimulationComponent::UpdateSurfaceSimulator()
{
	m_bUsingClientsideSimulation = false;
	m_physSurfaceSim = nullptr;
	if(cvSimShared->GetBool() == true) {
		auto *cent = static_cast<const SBaseEntity &>(GetEntity()).GetClientsideEntity();
		if(cent != nullptr) {
			auto *surfSim = GetSurfaceSimulator();
			if(surfSim != nullptr) {
				m_bUsingClientsideSimulation = true;
				m_physSurfaceSim = surfSim->shared_from_this();
				auto hEnt = cent->GetHandle();
				m_cbClientSimulatorUpdate = BindEventUnhandled(EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED, [this, hEnt](std::reference_wrapper<pragma::ComponentEvent> evData) {
					if(hEnt.valid() == false)
						return;
					auto *surfSim = GetSurfaceSimulator();
					m_physSurfaceSim = (surfSim != nullptr) ? surfSim->shared_from_this() : nullptr;
				});
				return;
			}
		}
	}
	ReloadSurfaceSimulator();
}
