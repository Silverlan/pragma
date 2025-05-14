/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_health_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;

void SHealthComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<unsigned short>(GetHealth()); }
void SHealthComponent::SetHealth(unsigned short health)
{
	BaseHealthComponent::SetHealth(health);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<unsigned short>(health);
	server->SendPacket("ent_sethealth", p, pragma::networking::Protocol::SlowReliable);
}
void SHealthComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
