// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.health;

import :entities;
import :server_state;

using namespace pragma;

void SHealthComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<unsigned short>(GetHealth()); }
void SHealthComponent::SetHealth(unsigned short health)
{
	BaseHealthComponent::SetHealth(health);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<unsigned short>(health);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETHEALTH, p, networking::Protocol::SlowReliable);
}
void SHealthComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
