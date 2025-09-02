// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.server.entities.components.liquid_buoyancy;

using namespace pragma;

void SBuoyancyComponent::Initialize() { BaseBuoyancyComponent::Initialize(); }

void SBuoyancyComponent::InitializeLuaObject(lua_State *l) { return BaseBuoyancyComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SBuoyancyComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
