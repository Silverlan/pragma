// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.effects.particle_system;

import :server_state;

using namespace pragma;

void SParticleSystemComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->WriteString(m_particleFile);
	packet->WriteString(m_particleName);
}

void SParticleSystemComponent::SetContinuous(bool b)
{
	BaseEnvParticleSystemComponent::SetContinuous(b);

	NetPacket p;
	networking::write_entity(p, &GetEntity());
	p->Write<bool>(b);
	ServerState::Get()->SendPacket(networking::net_messages::client::ENV_PRTSYS_SETCONTINUOUS, p, networking::Protocol::SlowReliable);
}

void SParticleSystemComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void EnvParticleSystem::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SParticleSystemComponent>();
}
