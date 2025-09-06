// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include "pragma/game/s_game.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/physics/phys_water_surface_simulator.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.liquid;

import pragma.server.entities.components;

using namespace pragma;

extern DLLSERVER SGame *s_game;

SLiquidComponent::SLiquidComponent(BaseEntity &ent) : BaseFuncLiquidComponent(ent) {}
SLiquidComponent::~SLiquidComponent()
{
	if(m_cbGameInitialized.IsValid())
		m_cbGameInitialized.Remove();
}

void SLiquidComponent::Initialize()
{
	BaseFuncLiquidComponent::Initialize();

	BindEventUnhandled(SModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		// TODO: Move this to shared
		auto &ent = GetEntity();
		if(ent.IsSpawned() == false || m_cbGameInitialized.IsValid() == true)
			return;
		InitializeWaterSurface();

		/*if(IsShared() == false)
			return;
		NetPacket packet {};
		packet->Write<Vector3>(m_waterPlane.GetNormal());
		packet->Write<double>(m_waterPlane.GetDistance());
		SendNetEventTCP(m_netEvSetWaterPlane,packet);*/
	});
	SetTickPolicy(TickPolicy::Always); // TODO
}

void SLiquidComponent::OnEntitySpawn()
{
	BaseFuncLiquidComponent::OnEntitySpawn();
	// TODO: Move this to shared
	auto &ent = GetEntity();
	auto *game = ent.GetNetworkState()->GetGameState();
	if(game->IsMapLoaded() == false) {
		// Need to wait until the game is initialized, to be sure the entity exists clientside
		m_cbGameInitialized = game->AddCallback("OnMapLoaded", FunctionCallback<void>::Create([this]() { InitializeWaterSurface(); }));
		return;
	}
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		return;
	InitializeWaterSurface();
}

void SLiquidComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}

///////////////

void SLiquidComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncWater::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLiquidComponent>();
}
