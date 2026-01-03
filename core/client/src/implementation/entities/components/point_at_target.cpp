// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_at_target;
import :networking.util;

using namespace pragma;

void CPointAtTargetComponent::ReceiveData(NetPacket &packet)
{
	auto hEnt = GetHandle();
	networking::read_unique_entity(packet, [this, hEnt](ecs::BaseEntity *ent) {
		if(hEnt.expired())
			return;
		SetPointAtTarget(ent);
	});
}
void CPointAtTargetComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

Bool CPointAtTargetComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetPointAtTarget) {
		auto *ent = networking::read_entity(packet);
		SetPointAtTarget(ent);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
