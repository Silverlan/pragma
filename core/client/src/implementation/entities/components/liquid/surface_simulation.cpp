// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/console/helper.hpp"

module pragma.client;

import :entities.components.liquid_surface_simulation;
import :entities.components.liquid_control;
import :entities.components.water_surface;
import :game;

import :physics;

using namespace pragma;

static std::vector<CLiquidSurfaceSimulationComponent *> s_waterEntities = {};
namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>(
	  "cl_water_surface_simulation_spacing", +[](NetworkState *, const console::ConVar &, int, int val) {
		  for(auto *entWater : s_waterEntities)
			  entWater->ReloadSurfaceSimulator();
	  });
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<bool>(
	  "cl_water_surface_simulation_enable_gpu_acceleration", +[](NetworkState *, const console::ConVar &, bool, bool val) {
		  for(auto *entWater : s_waterEntities)
			  entWater->ReloadSurfaceSimulator();
	  });
}

CLiquidSurfaceSimulationComponent::CLiquidSurfaceSimulationComponent(ecs::BaseEntity &ent) : BaseLiquidSurfaceSimulationComponent(ent) { s_waterEntities.push_back(this); }
CLiquidSurfaceSimulationComponent::~CLiquidSurfaceSimulationComponent()
{
	auto it = std::find(s_waterEntities.begin(), s_waterEntities.end(), this);
	if(it == s_waterEntities.end())
		return;
	s_waterEntities.erase(it);
}

void CLiquidSurfaceSimulationComponent::Initialize()
{
	BaseLiquidSurfaceSimulationComponent::Initialize();

	BindEventUnhandled(cLiquidControlComponent::EVENT_ON_SPLASH, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &splashInfo = static_cast<CEOnSplash &>(evData.get()).splashInfo;
		if(m_physSurfaceSim)
			static_cast<physics::CPhysWaterSurfaceSimulator &>(*m_physSurfaceSim).CreateSplash(splashInfo.origin, splashInfo.radius, splashInfo.force);
	});
	BindEventUnhandled(cLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &controlC = *GetEntity().GetComponent<CLiquidControlComponent>();
		if(m_physSurfaceSim) {
			m_physSurfaceSim->SetStiffness(controlC.GetStiffness());
			m_physSurfaceSim->SetPropagation(controlC.GetPropagation());
		}
	});
}

void CLiquidSurfaceSimulationComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CLiquidSurfaceSimulationComponent::ReceiveData(NetPacket &packet)
{
	auto height = packet->Read<float>();
	SetMaxWaveHeight(height);
}

std::shared_ptr<physics::PhysWaterSurfaceSimulator> CLiquidSurfaceSimulationComponent::InitializeSurfaceSimulator(const Vector2 &min, const Vector2 &max, float originY)
{
	auto controlC = GetEntity().GetComponent<CLiquidControlComponent>();
	return controlC.valid() ? pragma::util::make_shared<physics::CPhysWaterSurfaceSimulator>(min, max, originY, GetSpacing(), controlC->GetStiffness(), controlC->GetPropagation()) : nullptr;
}

void CLiquidSurfaceSimulationComponent::OnEntitySpawn()
{
	BaseLiquidSurfaceSimulationComponent::OnEntitySpawn();
	ReloadSurfaceSimulator();
}

CWaterSurface *CLiquidSurfaceSimulationComponent::GetSurfaceEntity() const { return static_cast<CWaterSurface *>(m_hWaterSurface.get()); }

void CLiquidSurfaceSimulationComponent::ReloadSurfaceSimulator()
{
	SetSpacing(get_cgame()->GetConVarInt("cl_water_surface_simulation_spacing"));
	BaseLiquidSurfaceSimulationComponent::ReloadSurfaceSimulator();
	if(m_hWaterSurface.valid())
		m_hWaterSurface->Remove();
	if(m_physSurfaceSim == nullptr)
		return;
	auto *entSurface = get_cgame()->CreateEntity<CWaterSurface>();
	if(entSurface == nullptr)
		return;
	auto &ent = GetEntity();
	ent.RemoveEntityOnRemoval(entSurface);
	m_hWaterSurface = entSurface->GetHandle();
	auto pWaterSurfComponent = entSurface->GetComponent<CWaterSurfaceComponent>();
	if(pWaterSurfComponent.valid()) {
		pWaterSurfComponent->SetWaterObject(this);
		pWaterSurfComponent->SetSurfaceSimulator(m_physSurfaceSim);
	}
	entSurface->Spawn();
}
