// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.quake;

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_quake, EnvQuake);

void SQuakeComponent::Initialize() { BaseEnvQuakeComponent::Initialize(); }

void SQuakeComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<UInt32>(m_quakeFlags);
	packet->Write<Float>(GetFrequency());
	packet->Write<Float>(GetAmplitude());
	packet->Write<Float>(GetRadius());
	packet->Write<Float>(GetDuration());
	packet->Write<Float>(GetFadeInDuration());
	packet->Write<Float>(GetFadeOutDuration());
}

void SQuakeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

extern ServerState *server;

void EnvQuake::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SQuakeComponent>();
}
