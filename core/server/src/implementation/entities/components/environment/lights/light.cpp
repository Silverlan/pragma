// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/shared_spawnflags.h"
#include "pragma/serverstate/serverstate.h"
#include <sharedutils/util_string.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

module pragma.server.entities.components.lights.base;

using namespace pragma;

extern ServerState *server;

void SLightComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<pragma::BaseEnvLightComponent::ShadowType>(GetShadowType());
	packet->Write<float>(m_falloffExponent);
	packet->Write<LightFlags>(m_lightFlags);
	packet->Write<float>(GetLightIntensity());
	packet->Write<LightIntensityType>(GetLightIntensityType());
}

void SLightComponent::SetShadowType(ShadowType type)
{
	BaseEnvLightComponent::SetShadowType(type);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write(type);
	ent.SendNetEvent(m_netEvSetShadowType, p, pragma::networking::Protocol::SlowReliable);
}

void SLightComponent::SetFalloffExponent(float falloffExponent)
{
	BaseEnvLightComponent::SetFalloffExponent(falloffExponent);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p {};
	p->Write<float>(falloffExponent);
	ent.SendNetEvent(m_netEvSetFalloffExponent, p, pragma::networking::Protocol::SlowReliable);
}

void SLightComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
