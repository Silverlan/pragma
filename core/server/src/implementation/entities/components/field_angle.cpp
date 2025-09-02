// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

module pragma.server.entities.components.field_angle;

using namespace pragma;

void SFieldAngleComponent::Initialize()
{
	BaseFieldAngleComponent::Initialize();
	FlagCallbackForRemoval(m_fieldAngle->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> val) {
		NetPacket p {};
		p->Write<float>(val.get());
		static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetFieldAngle, p, pragma::networking::Protocol::SlowReliable);
	}),
	  CallbackType::Component, this);
}
void SFieldAngleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SFieldAngleComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<float>(*m_fieldAngle); }
