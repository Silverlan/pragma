// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.field_angle;

import :entities;

using namespace pragma;

void SFieldAngleComponent::Initialize()
{
	BaseFieldAngleComponent::Initialize();
	FlagCallbackForRemoval(m_fieldAngle->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> val) {
		NetPacket p {};
		p->Write<float>(val.get());
		static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetFieldAngle, p, networking::Protocol::SlowReliable);
	}),
	  CallbackType::Component, this);
}
void SFieldAngleComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SFieldAngleComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<float>(*m_fieldAngle); }
