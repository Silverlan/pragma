// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.client;

import :entities.components.ownable;
using namespace pragma;

void COwnableComponent::ReceiveData(NetPacket &packet) { SetOwner(nwm::read_entity(packet)); }
void COwnableComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

Bool COwnableComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetOwner) {
		SetOwner(nwm::read_entity(packet));
		return true;
	}
	return false;
}
