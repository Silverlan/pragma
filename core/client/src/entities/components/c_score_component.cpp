/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_score_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void CScoreComponent::ReceiveData(NetPacket &packet) { *m_score = packet->Read<Score>(); }
void CScoreComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CScoreComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetScore)
		SetScore(packet->Read<Score>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
