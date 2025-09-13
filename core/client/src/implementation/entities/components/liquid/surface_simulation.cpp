// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/liquid/base_liquid_control_component.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.client.entities.components.liquid_surface_simulation;

import pragma.client.entities.components.liquid_control;
import pragma.client.entities.components.water_surface;

import pragma.client.physics;

extern CGame *c_game;

using namespace pragma;

static std::vector<CLiquidSurfaceSimulationComponent *> s_waterEntities = {};
REGISTER_CONVAR_CALLBACK_CL(cl_water_surface_simulation_spacing, [](NetworkState *, const ConVar &, int, int val) {
	for(auto *entWater : s_waterEntities)
		entWater->ReloadSurfaceSimulator();
});

REGISTER_CONVAR_CALLBACK_CL(cl_water_surface_simulation_enable_gpu_acceleration, [](NetworkState *, const ConVar &, bool, bool val) {
	for(auto *entWater : s_waterEntities)
		entWater->ReloadSurfaceSimulator();
});

CLiquidSurfaceSimulationComponent::CLiquidSurfaceSimulationComponent(BaseEntity &ent) : BaseLiquidSurfaceSimulationComponent(ent) { s_waterEntities.push_back(this); }
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

	BindEventUnhandled(CLiquidControlComponent::EVENT_ON_SPLASH, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &splashInfo = static_cast<CEOnSplash &>(evData.get()).splashInfo;
		if(m_physSurfaceSim)
			static_cast<CPhysWaterSurfaceSimulator &>(*m_physSurfaceSim).CreateSplash(splashInfo.origin, splashInfo.radius, splashInfo.force);
	});
	BindEventUnhandled(CLiquidControlComponent::EVENT_ON_PROPERTIES_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &controlC = *GetEntity().GetComponent<CLiquidControlComponent>();
		if(m_physSurfaceSim) {
			m_physSurfaceSim->SetStiffness(controlC.GetStiffness());
			m_physSurfaceSim->SetPropagation(controlC.GetPropagation());
		}
	});
}

void CLiquidSurfaceSimulationComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CLiquidSurfaceSimulationComponent::ReceiveData(NetPacket &packet)
{
	auto height = packet->Read<float>();
	SetMaxWaveHeight(height);
}

std::shared_ptr<PhysWaterSurfaceSimulator> CLiquidSurfaceSimulationComponent::InitializeSurfaceSimulator(const Vector2 &min, const Vector2 &max, float originY)
{
	auto controlC = GetEntity().GetComponent<CLiquidControlComponent>();
	return controlC.valid() ? std::make_shared<CPhysWaterSurfaceSimulator>(min, max, originY, GetSpacing(), controlC->GetStiffness(), controlC->GetPropagation()) : nullptr;
}

void CLiquidSurfaceSimulationComponent::OnEntitySpawn()
{
	BaseLiquidSurfaceSimulationComponent::OnEntitySpawn();
	ReloadSurfaceSimulator();
}

CWaterSurface *CLiquidSurfaceSimulationComponent::GetSurfaceEntity() const { return static_cast<CWaterSurface *>(m_hWaterSurface.get()); }

void CLiquidSurfaceSimulationComponent::ReloadSurfaceSimulator()
{
	SetSpacing(c_game->GetConVarInt("cl_water_surface_simulation_spacing"));
	BaseLiquidSurfaceSimulationComponent::ReloadSurfaceSimulator();
	if(m_hWaterSurface.valid())
		m_hWaterSurface->Remove();
	if(m_physSurfaceSim == nullptr)
		return;
	auto *entSurface = c_game->CreateEntity<CWaterSurface>();
	if(entSurface == nullptr)
		return;
	auto &ent = GetEntity();
	ent.RemoveEntityOnRemoval(entSurface);
	m_hWaterSurface = entSurface->GetHandle();
	auto pWaterSurfComponent = entSurface->GetComponent<pragma::CWaterSurfaceComponent>();
	if(pWaterSurfComponent.valid()) {
		pWaterSurfComponent->SetWaterObject(this);
		pWaterSurfComponent->SetSurfaceSimulator(m_physSurfaceSim);
	}
	entSurface->Spawn();
}
