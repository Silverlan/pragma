// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/components/c_surface_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void CSurfaceComponent::ReceiveData(NetPacket &packet) { m_plane = packet->Read<Vector4>(); }
void CSurfaceComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CSurfaceComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetPlane)
		SetPlane(packet->Read<Vector4>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
