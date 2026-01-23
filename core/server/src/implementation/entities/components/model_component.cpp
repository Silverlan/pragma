// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.model;

import :entities;
import :model_manager;
import :server_state;

using namespace pragma;

void SModelComponent::Initialize() { BaseModelComponent::Initialize(); }
void SModelComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SModelComponent::OnModelChanged(const std::shared_ptr<asset::Model> &model)
{
	BaseModelComponent::OnModelChanged(model);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->WriteString(GetModelName());
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_MODEL, p, networking::Protocol::SlowReliable);
	}
}

bool SModelComponent::SetBodyGroup(UInt32 groupId, UInt32 id)
{
	if(GetBodyGroup(groupId) == id)
		return true;
	auto r = BaseModelComponent::SetBodyGroup(groupId, id);
	if(r == false)
		return r;
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return r;
	NetPacket p;
	p->Write<UInt32>(groupId);
	p->Write<UInt32>(id);
	ent.SendNetEvent(m_netEvSetBodyGroup, p, networking::Protocol::SlowReliable);
	return r;
}

void SModelComponent::SetSkin(unsigned int skin)
{
	if(skin == GetSkin())
		return;
	BaseModelComponent::SetSkin(skin);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<unsigned int>(skin);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SKIN, p, networking::Protocol::SlowReliable);
}

void SModelComponent::SetMaxDrawDistance(float maxDist)
{
	if(maxDist == m_maxDrawDistance)
		return;
	BaseModelComponent::SetMaxDrawDistance(maxDist);
	NetPacket p {};
	p->Write<float>(m_maxDrawDistance);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvMaxDrawDist, p, networking::Protocol::SlowReliable);
}

void SModelComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	std::string mdl = GetModelName();
	packet->WriteString(mdl);
	packet->Write<unsigned int>(GetSkin());
	packet->Write<float>(m_maxDrawDistance);

	auto &bodyGroups = GetBodyGroups();
	packet->Write<uint32_t>(static_cast<uint32_t>(bodyGroups.size()));
	for(auto bg : bodyGroups)
		packet->Write<uint32_t>(bg);
}
