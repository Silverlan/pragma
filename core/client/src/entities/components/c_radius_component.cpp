// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/components/c_radius_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void CRadiusComponent::ReceiveData(NetPacket &packet) { SetRadius(packet->Read<float>()); }
void CRadiusComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CRadiusComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetRadius)
		SetRadius(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
