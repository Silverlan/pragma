/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/networking/s_nwm_util.h"
#include "pragma/entities/environment/lights/s_env_light_spot.h"
#include "pragma/entities/components/s_field_angle_component.hpp"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_spot, EnvLightSpot);

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
