#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

CBaseLightComponent::CBaseLightComponent(BaseEntity &ent)
	: BaseEnvLightComponent(ent)
{}

CBaseLightComponent::~CBaseLightComponent() {}

void CBaseLightComponent::Initialize()
{
	BaseEnvLightComponent::Initialize();
}

void CBaseLightComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvLightComponent::OnEntityComponentAdded(component);
}

util::EventReply CBaseLightComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEnvLightComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	return util::EventReply::Unhandled;
}

void CBaseLightComponent::ReceiveData(NetPacket &packet)
{
	m_shadowType = packet->Read<decltype(m_shadowType)>();
	SetFalloffExponent(packet->Read<float>());
}

BaseEntityComponent *CBaseLightComponent::GetLight(LightType &outType) const
{
	if(m_hLight.expired())
		outType = LightType::Invalid;
	else if(typeid(*m_hLight) == typeid(CLightSpotComponent))
		outType = LightType::Spot;
	else if(typeid(*m_hLight) == typeid(CLightPointComponent))
		outType = LightType::Point;
	else if(typeid(*m_hLight) == typeid(CLightDirectionalComponent))
		outType = LightType::Directional;
	else
		outType = LightType::Invalid;
	return m_hLight.get();
}
BaseEntityComponent *CBaseLightComponent::GetLight() const {return m_hLight.get();}

void CBaseLightComponent::SetLight(CLightSpotComponent &light) {InitializeLight(light);}
void CBaseLightComponent::SetLight(CLightPointComponent &light) {InitializeLight(light);}
void CBaseLightComponent::SetLight(CLightDirectionalComponent &light) {InitializeLight(light);}

void CBaseLightComponent::InitializeLight(BaseEntityComponent &component)
{
	auto pLightComponent = component.GetEntity().GetComponent<CLightComponent>();
	if(pLightComponent.expired())
	{
		m_hLight = {};
		return;
	}
	m_hLight = component.GetHandle();
}

Bool CBaseLightComponent::ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet)
{
	if(eventId == m_netEvSetShadowType)
		SetShadowType(packet->Read<BaseEnvLightComponent::ShadowType>());
	else if(eventId == m_netEvSetFalloffExponent)
		SetFalloffExponent(packet->Read<float>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId,packet);
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
