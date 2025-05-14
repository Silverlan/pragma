/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_field_angle_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

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
