/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/effects/s_env_particle_system.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_particle_system, EnvParticleSystem);

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
