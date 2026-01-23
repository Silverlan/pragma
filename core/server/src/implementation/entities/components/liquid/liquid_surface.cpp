// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.liquid_surface;

using namespace pragma;

void SLiquidSurfaceComponent::Initialize() { BaseLiquidSurfaceComponent::Initialize(); }

void SLiquidSurfaceComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidSurfaceComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
