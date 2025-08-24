// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.client.entities.components;

import :score;

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
