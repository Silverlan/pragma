/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void CColorComponent::ReceiveData(NetPacket &packet) { *m_color = packet->Read<Color>(); }
void CColorComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CColorComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetColor)
		SetColor(packet->Read<Color>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
