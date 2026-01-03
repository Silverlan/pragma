// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.name;

import :entities;
import :server_state;

using namespace pragma;

void SNameComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->WriteString(GetName()); }
void SNameComponent::SetName(std::string name)
{
	BaseNameComponent::SetName(name);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->WriteString(name);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETNAME, p, networking::Protocol::SlowReliable);
}
void SNameComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
