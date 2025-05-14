/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

CBaseLightComponent::CBaseLightComponent(BaseEntity &ent) : BaseEnvLightComponent(ent) {}

CBaseLightComponent::~CBaseLightComponent() {}

void CBaseLightComponent::Initialize() { BaseEnvLightComponent::Initialize(); }

void CBaseLightComponent::OnEntityComponentAdded(BaseEntityComponent &component) { BaseEnvLightComponent::OnEntityComponentAdded(component); }

util::EventReply CBaseLightComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvLightComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	return util::EventReply::Unhandled;
}

void CBaseLightComponent::ReceiveData(NetPacket &packet)
{
	m_shadowType = packet->Read<decltype(m_shadowType)>();
	SetFalloffExponent(packet->Read<float>());
	m_lightFlags = packet->Read<LightFlags>();

	auto lightIntensity = packet->Read<float>();
	auto lightIntensityType = packet->Read<LightIntensityType>();
	SetLightIntensity(lightIntensity, lightIntensityType);
}

Bool CBaseLightComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetShadowType)
		SetShadowType(packet->Read<BaseEnvLightComponent::ShadowType>());
	else if(eventId == m_netEvSetFalloffExponent)
		SetFalloffExponent(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

void CBaseLightComponent::OnEntitySpawn()
{
	BaseEnvLightComponent::OnEntitySpawn();
	if(m_hLight.valid())
		InitializeLightSource();
}
void CBaseLightComponent::InitializeLightSource()
{
	//auto &scene = c_game->GetScene();
	//scene->AddLight(m_light.get()); // prosper TODO
}

/////////////

void CEnvLight::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightComponent>();
}
