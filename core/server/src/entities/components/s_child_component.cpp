/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_child_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/networking/s_nwm_util.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void SChildComponent::Initialize() { BaseChildComponent::Initialize(); }
void SChildComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SChildComponent::OnParentChanged(BaseEntity *parent)
{
	NetPacket p;
	nwm::write_entity(p, parent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetParent, p, pragma::networking::Protocol::SlowReliable);
}

void SChildComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	auto *parent = GetParentEntity();
	if(parent)
		nwm::write_unique_entity(packet, parent);
	else
		nwm::write_unique_entity(packet, nullptr);
}
