// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.server.entities.components.liquid_volume;

using namespace pragma;

void SLiquidVolumeComponent::Initialize() { BaseLiquidVolumeComponent::Initialize(); }

void SLiquidVolumeComponent::InitializeLuaObject(lua_State *l) { return BaseLiquidVolumeComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidVolumeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
