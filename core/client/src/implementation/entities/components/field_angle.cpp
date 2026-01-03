// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.field_angle;
using namespace pragma;

void CFieldAngleComponent::ReceiveData(NetPacket &packet) { SetFieldAngle(packet->Read<float>()); }
void CFieldAngleComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CFieldAngleComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetFieldAngle)
		SetFieldAngle(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
