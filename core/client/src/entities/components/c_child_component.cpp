/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_child_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include "pragma/networking/c_nwm_util.h"

using namespace pragma;

void CChildComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CChildComponent::OnRemove() { BaseChildComponent::OnRemove(); }

bool CChildComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetParent) {
		auto *ent = nwm::read_entity(packet);
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
	nwm::read_unique_entity(packet, [this, hThis](BaseEntity *ent) {
		if(hThis.expired())
			return;
		if(ent)
			SetParent(*ent);
		else
			SetParent({});
	});
}
