// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.score;
using namespace pragma;

void CScoreComponent::ReceiveData(NetPacket &packet) { *m_score = packet->Read<Score>(); }
void CScoreComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CScoreComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetScore)
		SetScore(packet->Read<Score>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
