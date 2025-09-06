// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

module pragma.server.entities.components.surface;

import pragma.server.entities.base;
import pragma.server.server_state;

using namespace pragma;

void SSurfaceComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<Vector4>(GetPlane().ToVector4()); }
void SSurfaceComponent::SetPlane(const umath::Plane &plane)
{
	BaseSurfaceComponent::SetPlane(plane);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write<Vector4>(plane.ToVector4());
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetPlane, p, pragma::networking::Protocol::SlowReliable);
}
void SSurfaceComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
