// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_client.h"

module pragma.client;


import :entities.components.liquid_volume;
using namespace pragma;

void CLiquidVolumeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CLiquidVolumeComponent::ReceiveData(NetPacket &packet) {}
