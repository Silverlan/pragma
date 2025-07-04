// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/components/c_point_at_target_component.hpp"
#include "pragma/networking/c_nwm_util.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void CPointAtTargetComponent::ReceiveData(NetPacket &packet)
{
	auto hEnt = GetHandle();
	nwm::read_unique_entity(packet, [this, hEnt](BaseEntity *ent) {
		if(hEnt.expired())
			return;
		SetPointAtTarget(ent);
	});
}
void CPointAtTargetComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

Bool CPointAtTargetComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetPointAtTarget) {
		auto *ent = nwm::read_entity(packet);
		SetPointAtTarget(ent);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
