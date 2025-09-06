// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

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
	server->SendPacket("env_prtsys_setcontinuous", p, pragma::networking::Protocol::SlowReliable);
}

void SParticleSystemComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void EnvParticleSystem::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SParticleSystemComponent>();
}
