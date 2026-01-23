// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.render;

import :entities.base;
import :server_state;

using namespace pragma;

void SRenderComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) { packet->Write<decltype(m_renderFlags)>(m_renderFlags); }
void SRenderComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SRenderComponent::SetUnlit(bool b)
{
	BaseRenderComponent::SetUnlit(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<bool>(b);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETUNLIT, p, networking::Protocol::SlowReliable);
}
void SRenderComponent::SetCastShadows(bool b)
{
	BaseRenderComponent::SetCastShadows(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<bool>(b);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETCASTSHADOWS, p, networking::Protocol::SlowReliable);
}
