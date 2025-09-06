// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include "pragma/entities/s_baseentity.h"

module pragma.server.entities.components.lights.spot;

import pragma.server.entities.components.field_angle;
import pragma.server.entities.components.lights.base;

extern DLLSERVER ServerState *server;

using namespace pragma;

void SLightSpotComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<float>(*m_blendFraction);
	packet->Write<float>(*m_coneStartOffset);
}

void SLightSpotComponent::SetConeStartOffset(float offset)
{
	BaseEnvLightSpotComponent::SetConeStartOffset(offset);
	NetPacket p {};
	p->Write<float>(offset);
	static_cast<SBaseEntity &>(GetEntity()).SendNetEvent(m_netEvSetConeStartOffset, p, pragma::networking::Protocol::SlowReliable);
}

void SLightSpotComponent::SetOuterConeAngle(float ang)
{
	BaseEnvLightSpotComponent::SetOuterConeAngle(ang);
	auto &ent = GetEntity();
	if(!ent.IsSpawned())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<float>(ang);
	server->SendPacket("env_light_spot_outercutoff_angle", p, pragma::networking::Protocol::SlowReliable);
}

void SLightSpotComponent::SetBlendFraction(float ang)
{
	BaseEnvLightSpotComponent::SetBlendFraction(ang);
	auto &ent = GetEntity();
	if(!ent.IsSpawned())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<float>(ang);
	server->SendPacket("env_light_spot_innercutoff_angle", p, pragma::networking::Protocol::SlowReliable);
}

void SLightSpotComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SLightSpotComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(SLightComponent))
		static_cast<SLightComponent &>(component).SetLight(*this);
	else if(typeid(component) == typeid(SFieldAngleComponent))
		SetFieldAngleComponent(static_cast<SFieldAngleComponent &>(component));
}

///////////

void EnvLightSpot::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightComponent>();
	AddComponent<SLightSpotComponent>();
}
