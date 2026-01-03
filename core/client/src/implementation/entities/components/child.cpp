// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.child;
import :networking.util;

using namespace pragma;

void CChildComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CChildComponent::OnRemove() { BaseChildComponent::OnRemove(); }

bool CChildComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetParent) {
		auto *ent = networking::read_entity(packet);
		if(ent)
			SetParent(*ent);
		else
			SetParent({});
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

void CChildComponent::ReceiveData(NetPacket &packet)
{
	auto hThis = GetHandle();
	networking::read_unique_entity(packet, [this, hThis](ecs::BaseEntity *ent) {
		if(hThis.expired())
			return;
		if(ent)
			SetParent(*ent);
		else
			SetParent({});
	});
}
