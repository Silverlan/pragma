// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.prop.base;

using namespace pragma;

void PropComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PropComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<float>(m_kvMass); }
