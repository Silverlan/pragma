// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.color;

import pragma.server.entities;

using namespace pragma;

void SColorComponent::Initialize()
{
	BaseColorComponent::Initialize();
	FlagCallbackForRemoval(m_color->AddCallback([this](std::reference_wrapper<const Vector4> oldVal, std::reference_wrapper<const Vector4> val) {
		NetPacket p {};
		p->Write<Vector4>(val);
		static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetColor, p, pragma::networking::Protocol::SlowReliable);
	}),
	  CallbackType::Component, this);
}

void SColorComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SColorComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<Vector4>(*m_color); }
