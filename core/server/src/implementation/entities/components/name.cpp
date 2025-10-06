// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.name;

import pragma.server.entities;
import pragma.server.server_state;

using namespace pragma;

void SNameComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->WriteString(GetName()); }
void SNameComponent::SetName(std::string name)
{
	BaseNameComponent::SetName(name);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->WriteString(name);
	ServerState::Get()->SendPacket("ent_setname", p, pragma::networking::Protocol::SlowReliable);
}
void SNameComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
