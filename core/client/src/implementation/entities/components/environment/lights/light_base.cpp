// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.lights.light;
import :entities.components.radius;
import :game;

using namespace pragma;

CBaseLightComponent::CBaseLightComponent(ecs::BaseEntity &ent) : BaseEnvLightComponent(ent) {}

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

Bool CBaseLightComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetShadowType)
		SetShadowType(packet->Read<ShadowType>());
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
	//auto &scene = pragma::get_cgame()->GetScene();
	//scene->AddLight(m_light.get()); // prosper TODO
}

/////////////

void CEnvLight::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightComponent>();
}
