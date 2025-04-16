/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_ownable_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

void SOwnableComponent::Initialize() { BaseOwnableComponent::Initialize(); }
void SOwnableComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SOwnableComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { nwm::write_entity(packet, *m_owner); }

void SOwnableComponent::SetOwner(BaseEntity *owner)
{
	BaseOwnableComponent::SetOwner(owner);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	nwm::write_entity(p, *m_owner);
	ent.SendNetEvent(m_netEvSetOwner, p, pragma::networking::Protocol::SlowReliable);
}
