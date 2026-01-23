// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.surface;
using namespace pragma;

void CSurfaceComponent::ReceiveData(NetPacket &packet) { m_plane = packet->Read<Vector4>(); }
void CSurfaceComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CSurfaceComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetPlane)
		SetPlane(packet->Read<Vector4>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
