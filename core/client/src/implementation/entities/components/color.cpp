// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.color;
using namespace pragma;

void CColorComponent::ReceiveData(NetPacket &packet) { *m_color = packet->Read<Vector4>(); }
void CColorComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CColorComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetColor)
		SetColor(packet->Read<Vector4>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
