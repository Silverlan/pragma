// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.client.entities.components.health;

using namespace pragma;

void CHealthComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CHealthComponent::ReceiveData(NetPacket &packet) { *m_health = packet->Read<unsigned short>(); }
