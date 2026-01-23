// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.child;

import :entities;
import :networking.util;

using namespace pragma;

void SChildComponent::Initialize() { BaseChildComponent::Initialize(); }
void SChildComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SChildComponent::OnParentChanged(ecs::BaseEntity *parent)
{
	NetPacket p;
	networking::write_entity(p, parent);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetParent, p, networking::Protocol::SlowReliable);
}

void SChildComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	auto *parent = GetParentEntity();
	if(parent)
		networking::write_unique_entity(packet, parent);
	else
		networking::write_unique_entity(packet, nullptr);
}
