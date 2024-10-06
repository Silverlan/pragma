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
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <udm.hpp>
#include <algorithm>

using namespace pragma;

ComponentEventId BaseEnvLightComponent::EVENT_CALC_LIGHT_DIRECTION_TO_POINT = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseEnvLightComponent::EVENT_CALC_LIGHT_INTENSITY_AT_POINT = pragma::INVALID_COMPONENT_ID;
void BaseEnvLightComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_CALC_LIGHT_DIRECTION_TO_POINT = registerEvent("CALC_LIGHT_DIRECTION_TO_POINT", ComponentEventInfo::Type::Broadcast);
	EVENT_CALC_LIGHT_INTENSITY_AT_POINT = registerEvent("CALC_LIGHT_INTENSITY_AT_POINT", ComponentEventInfo::Type::Broadcast);
}
void BaseEnvLightComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseEnvLightComponent;

	using TFalloffExponent = float;
	registerMember(create_component_member_info<T, TFalloffExponent, static_cast<void (T::*)(TFalloffExponent)>(&T::SetFalloffExponent), static_cast<TFalloffExponent (T::*)() const>(&T::GetFalloffExponent)>("falloffExponent", 1.f));

	{
		using TIntensity = float;
		auto memberInfo = create_component_member_info<T, TIntensity, static_cast<void (T::*)(TIntensity)>(&T::SetLightIntensity), static_cast<TIntensity (T::*)() const>(&T::GetLightIntensity)>("intensity", 1.f, AttributeSpecializationType::LightIntensity);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}

	{
		using TIntensityType = LightIntensityType;
		auto memberInfo = create_component_member_info<T, TIntensityType, static_cast<void (T::*)(TIntensityType)>(&T::SetLightIntensityType), static_cast<TIntensityType (T::*)() const>(&T::GetLightIntensityType)>("intensityType", LightIntensityType::Candela);
		registerMember(std::move(memberInfo));
	}

	{
		using TCastShadows = bool;
		auto memberInfo = create_component_member_info<T, TCastShadows, [](const ComponentMemberInfo &, T &c, bool castShadows) { c.SetShadowType(castShadows ? ShadowType::Full : ShadowType::None); },
		  [](const ComponentMemberInfo &, T &c, TCastShadows &value) { value = c.GetShadowType() != ShadowType::None; }>("castShadows", true);
		registerMember(std::move(memberInfo));
	}

	{
		using TBaked = bool;
		auto memberInfo = create_component_member_info<T, TBaked, &BaseEnvLightComponent::SetBaked, &BaseEnvLightComponent::IsBaked>("baked", false);
		registerMember(std::move(memberInfo));
	}
}
bool BaseEnvLightComponent::IsBaked() const { return umath::is_flag_set(m_lightFlags, LightFlags::BakedLightSource); }
void BaseEnvLightComponent::SetBaked(bool baked) { umath::set_flag(m_lightFlags, LightFlags::BakedLightSource, baked); }
std::string BaseEnvLightComponent::LightIntensityTypeToString(LightIntensityType type)
{
	switch(type) {
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

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "distance", false))
			GetEntity().SetKeyValue("radius", kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "lightcolor", false))
			GetEntity().SetKeyValue("color", kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "light_intensity", false))
			SetLightIntensity(ustring::to_float(kvData.value));
		else if(ustring::compare<std::string>(kvData.key, "light_intensity_type", false))
			SetLightIntensityType(static_cast<LightIntensityType>(ustring::to_int(kvData.value)));
		else if(ustring::compare<std::string>(kvData.key, "falloff_exponent", false))
			SetFalloffExponent(util::to_float(kvData.value));
		else if(ustring::compare<std::string>(kvData.key, "light_flags", false))
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

void BaseEnvLightComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["lightType"] = m_lightType;
	udm["shadowType"] = m_shadowType;
	udm["falloffExponent"] = m_falloffExponent;
	udm["lightIntensityType"] = m_lightIntensityType;
	udm["lightIntensity"] = m_lightIntensity;
}
void BaseEnvLightComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	udm["lightType"](m_lightType);
	udm["shadowType"](m_shadowType);
	udm["falloffExponent"](m_falloffExponent);
	udm["lightIntensityType"](m_lightIntensityType);
	udm["lightIntensity"](m_lightIntensity);
}
void BaseEnvLightComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto flags = GetEntity().GetSpawnFlags();
	if(flags & umath::to_integral(SpawnFlag::DontCastShadows))
		m_shadowType = ShadowType::None;
}
void BaseEnvLightComponent::SetLight(BaseEnvLightSpotComponent &light)
{
	m_lightType = pragma::LightType::Spot;
	if(m_lightIntensityType == LightIntensityType::Lux)
		SetLightIntensity(0.f, LightIntensityType::Lumen);
	InitializeLight(light);
}
void BaseEnvLightComponent::SetLight(BaseEnvLightPointComponent &light)
{
	m_lightType = pragma::LightType::Point;
	if(m_lightIntensityType == LightIntensityType::Lux)
		SetLightIntensity(0.f, LightIntensityType::Lumen);
	InitializeLight(light);
}
void BaseEnvLightComponent::SetLight(BaseEnvLightDirectionalComponent &light)
{
	m_lightType = pragma::LightType::Directional;
	if(m_lightIntensityType != LightIntensityType::Lux)
		SetLightIntensity(0.f, LightIntensityType::Lux);
	InitializeLight(light);
}
BaseEntityComponent *BaseEnvLightComponent::GetLight(pragma::LightType &outType) const
{
	outType = m_lightType;
	return const_cast<BaseEntityComponent *>(m_hLight.get());
}
BaseEntityComponent *BaseEnvLightComponent::GetLight() const { return const_cast<BaseEntityComponent *>(m_hLight.get()); }
void BaseEnvLightComponent::InitializeLight(BaseEntityComponent &component) { m_hLight = component.GetHandle(); }
void BaseEnvLightComponent::SetLightIntensityType(LightIntensityType type) { m_lightIntensityType = type; }
BaseEnvLightComponent::LightIntensityType BaseEnvLightComponent::GetLightIntensityType() const { return m_lightIntensityType; }
void BaseEnvLightComponent::SetLightIntensity(float intensity, LightIntensityType type)
{
	if(m_lightType == pragma::LightType::Directional && type != LightIntensityType::Lux) {
		Con::cwar << "Attempted to use intensity type " << LightIntensityTypeToString(type) << " for a directional light source. This is not allowed!" << Con::endl;
		return;
	}
	if((m_lightType == pragma::LightType::Point || m_lightType == pragma::LightType::Spot) && type == LightIntensityType::Lux) {
		Con::cwar << "Attempted to use intensity type " << LightIntensityTypeToString(type) << " for a point or spot light source. This is not allowed!" << Con::endl;
		return;
	}
	m_lightIntensity = intensity;
	SetLightIntensityType(type);
}
void BaseEnvLightComponent::SetLightIntensity(float intensity) { SetLightIntensity(intensity, GetLightIntensityType()); }
float BaseEnvLightComponent::GetLightIntensity() const { return m_lightIntensity; }
Candela BaseEnvLightComponent::GetLightIntensityCandela(float intensity, LightIntensityType type, std::optional<float> outerConeAngle)
{
	switch(type) {
	case LightIntensityType::Candela:
		return intensity;
	case LightIntensityType::Lumen:
		{
			auto angle = outerConeAngle.has_value() ? *outerConeAngle : 360.f;
			return ulighting::lumens_to_candela(intensity, umath::cos(umath::deg_to_rad(angle / 2.f)));
		}
	case LightIntensityType::Lux:
		// TODO
		break;
	}
	return intensity;
}
Lumen BaseEnvLightComponent::GetLightIntensityLumen(float intensity, LightIntensityType type, std::optional<float> outerConeAngle)
{
	switch(type) {
	case LightIntensityType::Lumen:
		return intensity;
	case LightIntensityType::Candela:
		{
			auto angle = outerConeAngle.has_value() ? *outerConeAngle : 360.f;
			return ulighting::candela_to_lumens(intensity, umath::cos(umath::deg_to_rad(angle / 2.f)));
		}
	case LightIntensityType::Lux:
		// TODO
		break;
	}
	return intensity;
}
Candela BaseEnvLightComponent::GetLightIntensityCandela() const
{
	auto *spotLightC = static_cast<BaseEnvLightSpotComponent *>(GetEntity().FindComponent("light_spot").get());
	auto angle = spotLightC ? spotLightC->GetOuterConeAngle() : 360.f;
	return GetLightIntensityCandela(GetLightIntensity(), GetLightIntensityType(), angle);
}
Lumen BaseEnvLightComponent::GetLightIntensityLumen() const
{
	auto *spotLightC = static_cast<BaseEnvLightSpotComponent *>(GetEntity().FindComponent("light_spot").get());
	auto angle = spotLightC ? spotLightC->GetOuterConeAngle() : 360.f;
	return GetLightIntensityLumen(GetLightIntensity(), GetLightIntensityType(), angle);
}
float BaseEnvLightComponent::CalcLightIntensityAtPoint(const Vector3 &pos) const
{
	CECalcLightIntensityAtPoint ev {pos};
	BroadcastEvent(EVENT_CALC_LIGHT_INTENSITY_AT_POINT, ev);
	return ev.intensity;
}
Vector3 BaseEnvLightComponent::CalcLightDirectionToPoint(const Vector3 &pos) const
{
	CECalcLightDirectionToPoint ev {pos};
	BroadcastEvent(EVENT_CALC_LIGHT_DIRECTION_TO_POINT, ev);
	return ev.direction;
}
BaseEnvLightComponent::ShadowType BaseEnvLightComponent::GetShadowType() const { return m_shadowType; }
BaseEnvLightComponent::ShadowType BaseEnvLightComponent::GetEffectiveShadowType() const { return IsBaked() ? ShadowType ::None : GetShadowType(); }
void BaseEnvLightComponent::SetShadowType(ShadowType type) { m_shadowType = type; }
float BaseEnvLightComponent::GetFalloffExponent() const { return m_falloffExponent; }
void BaseEnvLightComponent::SetFalloffExponent(float falloffExponent) { m_falloffExponent = falloffExponent; }

float BaseEnvLightComponent::CalcDistanceFalloff(const Vector3 &lightPos, const Vector3 &point, std::optional<float> radius)
{
	auto dist = uvec::distance(point, lightPos);
	dist = pragma::units_to_metres(dist);
	if(radius.has_value())
		return ulighting::calc_light_falloff(dist, pragma::units_to_metres(*radius));
	return ulighting::calc_light_falloff(dist);
}

//////////////

CECalcLightDirectionToPoint::CECalcLightDirectionToPoint(const Vector3 &pos) : pos {pos} {}
void CECalcLightDirectionToPoint::PushArguments(lua_State *l) { Lua::Push<Vector3>(l, pos); }

CECalcLightIntensityAtPoint::CECalcLightIntensityAtPoint(const Vector3 &pos) : pos {pos} {}
void CECalcLightIntensityAtPoint::PushArguments(lua_State *l) { Lua::Push<Vector3>(l, pos); }
