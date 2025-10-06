// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>

module pragma.server.entities.components.timescale;

using namespace pragma;

void SEnvTimescaleComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(m_kvTimescale);
	packet->Write<float>(m_kvInnerRadius);
	packet->Write<float>(m_kvOuterRadius);
}

void SEnvTimescaleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvTimescale::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SEnvTimescaleComponent>();
}
