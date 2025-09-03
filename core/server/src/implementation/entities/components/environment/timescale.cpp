// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.timescale;

using namespace pragma;

void SEnvTimescaleComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvTimescale);
	packet->Write<float>(m_kvInnerRadius);
	packet->Write<float>(m_kvOuterRadius);
}

LINK_ENTITY_TO_CLASS(env_timescale, EnvTimescale);

void SEnvTimescaleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvTimescale::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SEnvTimescaleComponent>();
}
