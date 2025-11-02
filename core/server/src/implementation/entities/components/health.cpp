// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_server.h"

module pragma.server;
import :entities.components.health;

import :entities;
import :server_state;

using namespace pragma;

void SHealthComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<unsigned short>(GetHealth()); }
void SHealthComponent::SetHealth(unsigned short health)
{
	BaseHealthComponent::SetHealth(health);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<unsigned short>(health);
	ServerState::Get()->SendPacket("ent_sethealth", p, pragma::networking::Protocol::SlowReliable);
}
void SHealthComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
