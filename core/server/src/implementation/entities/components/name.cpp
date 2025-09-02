// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <sharedutils/netpacket.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

module pragma.server.entities.components.name;

using namespace pragma;

extern DLLSERVER ServerState *server;

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
	server->SendPacket("ent_setname", p, pragma::networking::Protocol::SlowReliable);
}
void SNameComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
