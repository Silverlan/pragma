/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_render_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SRenderComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<decltype(m_renderFlags)>(m_renderFlags); }
void SRenderComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SRenderComponent::SetUnlit(bool b)
{
	BaseRenderComponent::SetUnlit(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<bool>(b);
	server->SendPacket("ent_setunlit", p, pragma::networking::Protocol::SlowReliable);
}
void SRenderComponent::SetCastShadows(bool b)
{
	BaseRenderComponent::SetCastShadows(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<bool>(b);
	server->SendPacket("ent_setcastshadows", p, pragma::networking::Protocol::SlowReliable);
}
