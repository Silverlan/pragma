// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.liquid_volume;

using namespace pragma;

void SLiquidVolumeComponent::Initialize() { BaseLiquidVolumeComponent::Initialize(); }

void SLiquidVolumeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLiquidVolumeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
