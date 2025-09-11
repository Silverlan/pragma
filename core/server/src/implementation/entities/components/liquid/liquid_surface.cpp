// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"

module pragma.server.entities.components.liquid_surface;

using namespace pragma;

void SLiquidSurfaceComponent::Initialize() { BaseLiquidSurfaceComponent::Initialize(); }

void SLiquidSurfaceComponent::InitializeLuaObject(lua_State *l) { return BaseLiquidSurfaceComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidSurfaceComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
