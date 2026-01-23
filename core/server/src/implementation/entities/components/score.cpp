// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.score;

import :entities.base;

using namespace pragma;

void SScoreComponent::Initialize()
{
	BaseScoreComponent::Initialize();
	FlagCallbackForRemoval(m_score->AddCallback([this](std::reference_wrapper<const Score> oldVal, std::reference_wrapper<const Score> val) {
		NetPacket p {};
		p->Write<Score>(val);
		static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetScore, p, networking::Protocol::SlowReliable);
	}),
	  CallbackType::Component, this);
}

void SScoreComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SScoreComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<Score>(*m_score); }
