/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light.h"
#include <sharedutils/util.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/entities/baseentity_events.hpp"
#include <algorithm>

using namespace pragma;


std::string BaseEnvLightComponent::LightIntensityTypeToString(LightIntensityType type)
{
	switch(type)
	{
	case LightIntensityType::Candela:
		return "Candela";
	case LightIntensityType::Lumen:
		return "Lumen";
	case LightIntensityType::Lux:
		return "Lux";
	}
	return "Unknown";
}
void BaseEnvLightComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"distance",false))
			GetEntity().SetKeyValue("radius",kvData.value);
		else if(ustring::compare(kvData.key,"lightcolor",false))
			GetEntity().SetKeyValue("color",kvData.value);
		else if(ustring::compare(kvData.key,"light_intensity",false))
			SetLightIntensity(ustring::to_float(kvData.value));
		else if(ustring::compare(kvData.key,"light_intensity_type",false))
			SetLightIntensityType(static_cast<LightIntensityType>(ustring::to_int(kvData.value)));
		else if(ustring::compare(kvData.key,"falloff_exponent",false))
			SetFalloffExponent(util::to_float(kvData.value));
		else if(ustring::compare(kvData.key,"light_flags",false))
			m_lightFlags = static_cast<LightFlags>(util::to_int(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("transform");
	ent.AddComponent("toggle");
	ent.AddComponent("color");
	m_netEvSetShadowType = SetupNetEvent("set_shadow_type");
	m_netEvSetFalloffExponent = SetupNetEvent("set_falloff_exponent");
}
void BaseEnvLightComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto flags = GetEntity().GetSpawnFlags();
	if(flags &umath::to_integral(SpawnFlag::DontCastShadows))
		m_shadowType = ShadowType::None;
}
void BaseEnvLightComponent::SetLight(BaseEnvLightSpotComponent &light)
{
	m_lightType = util::pragma::LightType::Spot;
	if(m_lightIntensityType == LightIntensityType::Lux)
		SetLightIntensity(0.f,LightIntensityType::Lumen);
	InitializeLight(light);
}
void BaseEnvLightComponent::SetLight(BaseEnvLightPointComponent &light)
{
	m_lightType = util::pragma::LightType::Point;
	if(m_lightIntensityType == LightIntensityType::Lux)
		SetLightIntensity(0.f,LightIntensityType::Lumen);
	InitializeLight(light);
}
void BaseEnvLightComponent::SetLight(BaseEnvLightDirectionalComponent &light)
{
	m_lightType = util::pragma::LightType::Directional;
	if(m_lightIntensityType != LightIntensityType::Lux)
		SetLightIntensity(0.f,LightIntensityType::Lux);
	InitializeLight(light);
}
BaseEntityComponent *BaseEnvLightComponent::GetLight(util::pragma::LightType &outType) const
{
	outType = m_lightType;
	return m_hLight.get();
}
BaseEntityComponent *BaseEnvLightComponent::GetLight() const {return m_hLight.get();}
void BaseEnvLightComponent::InitializeLight(BaseEntityComponent &component) {m_hLight = component.GetHandle();}
void BaseEnvLightComponent::SetLightIntensityType(LightIntensityType type) {m_lightIntensityType = type;}
BaseEnvLightComponent::LightIntensityType BaseEnvLightComponent::GetLightIntensityType() const {return m_lightIntensityType;}
void BaseEnvLightComponent::SetLightIntensity(float intensity,LightIntensityType type)
{
	if(m_lightType == util::pragma::LightType::Directional && type != LightIntensityType::Lux)
	{
		Con::cwar<<"WARNING: Attempted to use intensity type "<<LightIntensityTypeToString(type)<<" for a directional light source. This is not allowed!"<<Con::endl;
		return;
	}
	if((m_lightType == util::pragma::LightType::Point || m_lightType == util::pragma::LightType::Spot) && type == LightIntensityType::Lux)
	{
		Con::cwar<<"WARNING: Attempted to use intensity type "<<LightIntensityTypeToString(type)<<" for a point or spot light source. This is not allowed!"<<Con::endl;
		return;
	}
	m_lightIntensity = intensity;
	SetLightIntensityType(type);
}
void BaseEnvLightComponent::SetLightIntensity(float intensity) {SetLightIntensity(intensity,GetLightIntensityType());}
float BaseEnvLightComponent::GetLightIntensity() const {return m_lightIntensity;}
Candela BaseEnvLightComponent::GetLightIntensityCandela(float intensity,LightIntensityType type,std::optional<float> outerCutoffAngle)
{
	switch(type)
	{
	case LightIntensityType::Candela:
		return intensity;
	case LightIntensityType::Lumen:
	{
		auto angle = outerCutoffAngle.has_value() ? *outerCutoffAngle : 180.f; // Note: This is the HALF-angle, so we use 180 degree for point-lights
		return ulighting::lumens_to_candela(intensity,umath::cos(umath::deg_to_rad(angle)));
	}
	case LightIntensityType::Lux:
		// TODO
		break;
	}
	return intensity;
}
Lumen BaseEnvLightComponent::GetLightIntensityLumen(float intensity,LightIntensityType type,std::optional<float> outerCutoffAngle)
{
	switch(type)
	{
	case LightIntensityType::Lumen:
		return intensity;
	case LightIntensityType::Candela:
	{
		auto angle = outerCutoffAngle.has_value() ? *outerCutoffAngle : 180.f; // Note: This is the HALF-angle, so we use 180 degree for point-lights
		return ulighting::candela_to_lumens(intensity,umath::cos(umath::deg_to_rad(angle)));
	}
	case LightIntensityType::Lux:
		// TODO
		break;
	}
	return intensity;
}
Candela BaseEnvLightComponent::GetLightIntensityCandela() const
{
	auto *spotLightC = static_cast<BaseEnvLightSpotComponent*>(GetEntity().FindComponent("light_spot").get());
	auto angle = spotLightC ? spotLightC->GetOuterCutoffAngle() : 180.f; // Note: This is the HALF-angle, so we use 180 degree for point-lights
	return GetLightIntensityCandela(GetLightIntensity(),GetLightIntensityType(),angle);
}
Lumen BaseEnvLightComponent::GetLightIntensityLumen() const
{
	auto *spotLightC = static_cast<BaseEnvLightSpotComponent*>(GetEntity().FindComponent("light_spot").get());
	auto angle = spotLightC ? spotLightC->GetOuterCutoffAngle() : 180.f; // Note: This is the HALF-angle, so we use 180 degree for point-lights
	return GetLightIntensityLumen(GetLightIntensity(),GetLightIntensityType(),angle);
}
BaseEnvLightComponent::ShadowType BaseEnvLightComponent::GetShadowType() const {return m_shadowType;}
void BaseEnvLightComponent::SetShadowType(ShadowType type) {m_shadowType = type;}
float BaseEnvLightComponent::GetFalloffExponent() const {return m_falloffExponent;}
void BaseEnvLightComponent::SetFalloffExponent(float falloffExponent) {m_falloffExponent = falloffExponent;}

