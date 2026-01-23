// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.ownable;
using namespace pragma;

void COwnableComponent::ReceiveData(NetPacket &packet) { SetOwner(networking::read_entity(packet)); }
void COwnableComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

Bool COwnableComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetOwner) {
		SetOwner(networking::read_entity(packet));
		return true;
	}
	return false;
}
