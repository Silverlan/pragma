// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.liquid_surface_simulation;

import :console.register_commands;
import :console.util;
import :entities;

using namespace pragma;

static std::vector<SLiquidSurfaceSimulationComponent *> s_waterEntities = {};
namespace {
	auto _ = pragma::console::server::register_variable_listener<float>(
	  "sv_water_surface_simulation_shared", +[](NetworkState *, const console::ConVar &, float, float val) {
		  for(auto *entWater : s_waterEntities)
			  entWater->UpdateSurfaceSimulator();
	  });
};

SLiquidSurfaceSimulationComponent::SLiquidSurfaceSimulationComponent(ecs::BaseEntity &ent) : BaseLiquidSurfaceSimulationComponent(ent) { s_waterEntities.push_back(this); }
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
	BindEventUnhandled(baseSurfaceComponent::EVENT_ON_SURFACE_MESH_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdateSurfaceSimulator(); });
}

void SLiquidSurfaceSimulationComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidSurfaceSimulationComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<float>(m_kvMaxWaveHeight); }

static auto cvSimShared = console::get_server_con_var("sv_water_surface_simulation_shared");
bool SLiquidSurfaceSimulationComponent::ShouldSimulateSurface() const { return (BaseLiquidSurfaceSimulationComponent::ShouldSimulateSurface() == true && (cvSimShared->GetBool() == true || static_cast<const SBaseEntity &>(GetEntity()).GetClientsideEntity() == nullptr)) ? true : false; }

void SLiquidSurfaceSimulationComponent::OnTick(double dt)
{
	if(m_bUsingClientsideSimulation == true || m_physSurfaceSim == nullptr)
		return;
	auto *sim = static_cast<physics::PhysWaterSurfaceSimulator *>(m_physSurfaceSim.get());
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
				m_cbClientSimulatorUpdate = BindEventUnhandled(sLiquidSurfaceSimulationComponent::EVENT_ON_WATER_SURFACE_SIMULATOR_CHANGED, [this, hEnt](std::reference_wrapper<ComponentEvent> evData) {
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
