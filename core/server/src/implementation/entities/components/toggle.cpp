// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.toggle;

import pragma.server.entities;
import pragma.server.game;

using namespace pragma;

void SToggleComponent::TurnOn()
{
	if(IsTurnedOn())
		return;
	BaseToggleComponent::TurnOn();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<bool>(true);
	ent.SendNetEvent(m_netEvToggleState, p, pragma::networking::Protocol::SlowReliable);
}
void SToggleComponent::TurnOff()
{
	if(IsTurnedOn() == false)
		return;
	BaseToggleComponent::TurnOff();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<bool>(false);
	ent.SendNetEvent(m_netEvToggleState, p, pragma::networking::Protocol::SlowReliable);
}
void SToggleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SToggleComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<bool>(IsTurnedOn()); }
