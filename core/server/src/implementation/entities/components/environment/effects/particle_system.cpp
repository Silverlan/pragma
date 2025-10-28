// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

#include "stdafx_server.h"

module pragma.server.entities.components.effects.particle_system;

import pragma.server.server_state;

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
	nwm::write_entity(p, &GetEntity());
	p->Write<bool>(b);
	ServerState::Get()->SendPacket("env_prtsys_setcontinuous", p, pragma::networking::Protocol::SlowReliable);
}

void SParticleSystemComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void EnvParticleSystem::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SParticleSystemComponent>();
}
