/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_toggle_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

extern DLLSERVER SGame *s_game;

using namespace pragma;

void SToggleComponent::TurnOn()
{
	if(IsTurnedOn())
		return;
	BaseToggleComponent::TurnOn();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<bool>(true);
	ent.SendNetEvent(m_netEvToggleState, p, pragma::networking::Protocol::SlowReliable);
}
void SToggleComponent::TurnOff()
{
	if(IsTurnedOn() == false)
		return;
	BaseToggleComponent::TurnOff();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<bool>(false);
	ent.SendNetEvent(m_netEvToggleState, p, pragma::networking::Protocol::SlowReliable);
}
void SToggleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SToggleComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<bool>(IsTurnedOn()); }
