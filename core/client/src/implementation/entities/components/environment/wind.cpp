// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.client.entities.components.env_wind;

using namespace pragma;

void CWindComponent::ReceiveData(NetPacket &packet)
{
	auto windForce = packet->Read<Vector3>();
	SetWindForce(windForce);
}
void CWindComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void CEnvWind::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWindComponent>();
}
