// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.server;
import :entities.components.fog_controller;

import :server_state;

using namespace pragma;

void SFogControllerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void SFogControllerComponent::SetFogStart(float start)
{
	BaseEnvFogControllerComponent::SetFogStart(start);
	NetPacket p;
	nwm::write_entity(p, &GetEntity());
	p->Write<float>(m_kvFogStart);
	ServerState::Get()->SendPacket("env_fogcon_setstartdist", p, pragma::networking::Protocol::SlowReliable);
}
void SFogControllerComponent::SetFogEnd(float end)
{
	BaseEnvFogControllerComponent::SetFogEnd(end);
	NetPacket p;
	nwm::write_entity(p, &GetEntity());
	p->Write<float>(m_kvFogEnd);
	ServerState::Get()->SendPacket("env_fogcon_setenddist", p, pragma::networking::Protocol::SlowReliable);
}
void SFogControllerComponent::SetMaxDensity(float density)
{
	BaseEnvFogControllerComponent::SetMaxDensity(density);
	NetPacket p;
	nwm::write_entity(p, &GetEntity());
	p->Write<float>(m_kvMaxDensity);
	ServerState::Get()->SendPacket("env_fogcon_setmaxdensity", p, pragma::networking::Protocol::SlowReliable);
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
