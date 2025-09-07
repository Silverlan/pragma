// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.client.entities.components.toggle;

using namespace pragma;

void CToggleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CToggleComponent::ReceiveData(NetPacket &packet) { SetTurnedOn(packet->Read<bool>()); }
Bool CToggleComponent::ReceiveNetEvent(UInt32 eventId, NetPacket &packet)
{
	if(eventId == m_netEvToggleState) {
		auto bState = packet->Read<bool>();
		if(bState)
			TurnOn();
		else
			TurnOff();
		return true;
	}
	return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
}
