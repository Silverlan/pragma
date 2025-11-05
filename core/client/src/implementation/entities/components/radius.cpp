// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.client;

import :entities.components.radius;
using namespace pragma;

CRadiusComponent::CRadiusComponent(pragma::ecs::BaseEntity &ent) : BaseRadiusComponent(ent) {}
bool CRadiusComponent::ShouldTransmitNetData() const { return true; }
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
