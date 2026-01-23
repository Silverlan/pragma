// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.health;
using namespace pragma;

void CHealthComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CHealthComponent::ReceiveData(NetPacket &packet) { *m_health = packet->Read<unsigned short>(); }
