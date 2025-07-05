// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/entities/components/liquid/s_liquid_surface_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

void SLiquidSurfaceComponent::Initialize() { BaseLiquidSurfaceComponent::Initialize(); }

void SLiquidSurfaceComponent::InitializeLuaObject(lua_State *l) { return BaseLiquidSurfaceComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidSurfaceComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
