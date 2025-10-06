// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.point_at_target;

import pragma.server.entities.base;
import pragma.server.networking.util;

using namespace pragma;

void SPointAtTargetComponent::Initialize() { BasePointAtTargetComponent::Initialize(); }
void SPointAtTargetComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SPointAtTargetComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { nwm::write_unique_entity(packet, GetPointAtTarget()); }

void SPointAtTargetComponent::SetPointAtTarget(BaseEntity *ent)
{
	BasePointAtTargetComponent::SetPointAtTarget(ent);
	NetPacket p {};
	nwm::write_entity(p, ent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetPointAtTarget, p, pragma::networking::Protocol::SlowReliable);
}
