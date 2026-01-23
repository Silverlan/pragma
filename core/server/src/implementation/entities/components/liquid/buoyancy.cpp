// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.liquid_buoyancy;

using namespace pragma;

void SBuoyancyComponent::Initialize() { BaseBuoyancyComponent::Initialize(); }

void SBuoyancyComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SBuoyancyComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) {}
