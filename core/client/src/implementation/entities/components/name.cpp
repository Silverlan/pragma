// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.client.entities.components.name;

using namespace pragma;

void CNameComponent::ReceiveData(NetPacket &packet)
{
	std::string name = packet->ReadString();
	SetName(name);
}
void CNameComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
