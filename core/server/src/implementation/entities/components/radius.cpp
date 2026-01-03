// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.radius;

import :entities.base;

using namespace pragma;

void SRadiusComponent::Initialize()
{
	BaseRadiusComponent::Initialize();
	FlagCallbackForRemoval(m_radius->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> val) {
		NetPacket p {};
		p->Write<float>(val.get());
		static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetRadius, p, networking::Protocol::SlowReliable);
	}),
	  CallbackType::Component, this);
}
void SRadiusComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SRadiusComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<float>(*m_radius); }
