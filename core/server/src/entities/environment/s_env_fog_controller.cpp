/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_fog_controller.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_fog_controller, EnvFogController);

void SFogControllerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SFogControllerComponent::SetFogStart(float start)
{
	BaseEnvFogControllerComponent::SetFogStart(start);
	NetPacket p;
	nwm::write_entity(p, &GetEntity());
	p->Write<float>(m_kvFogStart);
	server->SendPacket("env_fogcon_setstartdist", p, pragma::networking::Protocol::SlowReliable);
}
void SFogControllerComponent::SetFogEnd(float end)
{
	BaseEnvFogControllerComponent::SetFogEnd(end);
	NetPacket p;
	nwm::write_entity(p, &GetEntity());
	p->Write<float>(m_kvFogEnd);
	server->SendPacket("env_fogcon_setenddist", p, pragma::networking::Protocol::SlowReliable);
}
void SFogControllerComponent::SetMaxDensity(float density)
{
	BaseEnvFogControllerComponent::SetMaxDensity(density);
	NetPacket p;
	nwm::write_entity(p, &GetEntity());
	p->Write<float>(m_kvMaxDensity);
	server->SendPacket("env_fogcon_setmaxdensity", p, pragma::networking::Protocol::SlowReliable);
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
