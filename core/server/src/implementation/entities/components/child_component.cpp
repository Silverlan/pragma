// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_server.h"

module pragma.server.entities.components.child;

import pragma.server.entities;
import pragma.server.networking.util;

using namespace pragma;

void SChildComponent::Initialize() { BaseChildComponent::Initialize(); }
void SChildComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SChildComponent::OnParentChanged(pragma::ecs::BaseEntity *parent)
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
