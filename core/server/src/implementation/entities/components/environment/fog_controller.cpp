// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.fog_controller;

import :server_state;

using namespace pragma;

void SFogControllerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SFogControllerComponent::SetFogStart(float start)
{
	BaseEnvFogControllerComponent::SetFogStart(start);
	NetPacket p;
	networking::write_entity(p, &GetEntity());
	p->Write<float>(m_kvFogStart);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENV_FOGCON_SETSTARTDIST, p, networking::Protocol::SlowReliable);
}
void SFogControllerComponent::SetFogEnd(float end)
{
	BaseEnvFogControllerComponent::SetFogEnd(end);
	NetPacket p;
	networking::write_entity(p, &GetEntity());
	p->Write<float>(m_kvFogEnd);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENV_FOGCON_SETENDDIST, p, networking::Protocol::SlowReliable);
}
void SFogControllerComponent::SetMaxDensity(float density)
{
	BaseEnvFogControllerComponent::SetMaxDensity(density);
	NetPacket p;
	networking::write_entity(p, &GetEntity());
	p->Write<float>(m_kvMaxDensity);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENV_FOGCON_SETMAXDENSITY, p, networking::Protocol::SlowReliable);
}
void SFogControllerComponent::SetFogType(util::FogType type) { BaseEnvFogControllerComponent::SetFogType(type); }
void SFogControllerComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<util::FogType>(m_kvFogType);
	packet->Write<float>(m_kvFogStart);
	packet->Write<float>(m_kvFogEnd);
	packet->Write<float>(m_kvMaxDensity);
}

///////////////

void EnvFogController::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFogControllerComponent>();
}
