// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ownable;

import :entities;

using namespace pragma;

void SOwnableComponent::Initialize() { BaseOwnableComponent::Initialize(); }
void SOwnableComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SOwnableComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { networking::write_entity(packet, *m_owner); }

void SOwnableComponent::SetOwner(ecs::BaseEntity *owner)
{
	BaseOwnableComponent::SetOwner(owner);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	networking::write_entity(p, *m_owner);
	ent.SendNetEvent(m_netEvSetOwner, p, networking::Protocol::SlowReliable);
}
