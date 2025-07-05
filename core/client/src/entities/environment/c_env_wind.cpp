// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_wind.hpp"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_wind, CEnvWind);

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
