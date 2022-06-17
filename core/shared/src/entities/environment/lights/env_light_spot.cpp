/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <sharedutils/util.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <algorithm>
#include <udm.hpp>

using namespace pragma;

umath::Degree BaseEnvLightSpotComponent::CalcInnerConeAngle(umath::Degree outerConeAngle,float blendFraction)
{
	return outerConeAngle *umath::clamp(1.f -blendFraction,0.f,1.f);
}
float BaseEnvLightSpotComponent::CalcBlendFraction(float outerConeAngle,float innerConeAngle)
{
	innerConeAngle = umath::clamp(innerConeAngle,0.f,outerConeAngle);
	return 1.f -(innerConeAngle /outerConeAngle);
}
void BaseEnvLightSpotComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
	using T = BaseEnvLightSpotComponent;

	{
		using TBlend = float;
		auto memberInfo = create_component_member_info<
			T,TBlend,
			static_cast<void(T::*)(TBlend)>(&T::SetBlendFraction),
			static_cast<TBlend(T::*)() const>(&T::GetBlendFraction)
		>("blendFraction",0.1f,AttributeSpecializationType::Fraction);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}
	
	using TConeAngle = float;
	{
		auto memberInfo = create_component_member_info<
			T,TConeAngle,
			static_cast<void(T::*)(TConeAngle)>(&T::SetOuterConeAngle),
			static_cast<TConeAngle(T::*)() const>(&T::GetOuterConeAngle)
		>("outerConeAngle",0.f,AttributeSpecializationType::Angle);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(179.99f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<
			T,TConeAngle,
			static_cast<void(T::*)(TConeAngle)>(&T::SetConeStartOffset),
			static_cast<TConeAngle(T::*)() const>(&T::GetConeStartOffset)
		>("coneStartOffset",0.f,AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
}
BaseEnvLightSpotComponent::BaseEnvLightSpotComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_blendFraction(util::FloatProperty::Create(0.1f)),
	m_outerConeAngle(util::FloatProperty::Create(0.f)),m_coneStartOffset(util::FloatProperty::Create(0.f))
{}
void BaseEnvLightSpotComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"outerconeangle",false))
			*m_outerConeAngle = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key,"outercutoff",false))
			*m_outerConeAngle = util::to_float(kvData.value) *2.f;
		else if(ustring::compare<std::string>(kvData.key,"blendfraction",false))
			*m_blendFraction = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key,"cone_start_offset",false))
			SetConeStartOffset(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("light");
	ent.AddComponent("radius");
	ent.AddComponent("point_at_target");
	m_netEvSetConeStartOffset = SetupNetEvent("set_cone_start_offset");
}

float BaseEnvLightSpotComponent::CalcConeFalloff(
	const Vector3 &lightPos,const Vector3 &lightDir,umath::Degree outerConeAngle,umath::Degree innerConeAngle,const Vector3 &point
)
{
	auto posFromWorldSpace = lightPos -point;
	auto dirToLight = posFromWorldSpace;
	uvec::normalize(&dirToLight);
	return ulighting::calc_cone_falloff(lightDir,dirToLight,outerConeAngle,innerConeAngle);
}

Candela BaseEnvLightSpotComponent::CalcIntensityFalloff(
	const Vector3 &lightPos,float radius,const Vector3 &lightDir,
	umath::Degree outerConeAngle,umath::Degree innerConeAngle,const Vector3 &point
)
{
	return BaseEnvLightComponent::CalcDistanceFalloff(lightPos,point,radius) *CalcConeFalloff(lightPos,lightDir,outerConeAngle,innerConeAngle,point);
}
Candela BaseEnvLightSpotComponent::CalcIntensityAtPoint(
	const Vector3 &lightPos,float radius,Candela intensity,const Vector3 &lightDir,
	umath::Degree outerConeAngle,umath::Degree innerConeAngle,const Vector3 &point
)
{
	intensity *= CalcIntensityFalloff(lightPos,radius,lightDir,outerConeAngle,innerConeAngle,point);
	return intensity;
}
util::EventReply BaseEnvLightSpotComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(eventId == BaseEnvLightComponent::EVENT_CALC_LIGHT_DIRECTION_TO_POINT)
	{
		auto &levData = static_cast<CECalcLightDirectionToPoint&>(evData);
		auto dir = levData.pos -GetEntity().GetPosition();
		uvec::normalize(&dir);
		levData.direction = dir;
		return util::EventReply::Handled;
	}
	else if(eventId == BaseEnvLightComponent::EVENT_CALC_LIGHT_INTENSITY_AT_POINT)
	{
		auto &levData = static_cast<CECalcLightIntensityAtPoint&>(evData);
		auto *cLight = dynamic_cast<pragma::BaseEnvLightComponent*>(GetEntity().FindComponent("light").get());
		if(cLight)
		{
			auto *radiusC = dynamic_cast<pragma::BaseRadiusComponent*>(GetEntity().FindComponent("radius").get());
			auto radius = radiusC ? radiusC->GetRadius() : 0.f;
			static_cast<CECalcLightIntensityAtPoint&>(evData).intensity = CalcIntensityAtPoint(
				GetEntity().GetPosition(),radius,cLight->GetLightIntensityCandela(),GetEntity().GetForward(),
				GetOuterConeAngle(),GetInnerConeAngle(),levData.pos
			);
		}
		return util::EventReply::Handled;
	}
	return BaseEntityComponent::HandleEvent(eventId,evData);
}

void BaseEnvLightSpotComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["blendFraction"] = **m_blendFraction;
	udm["outerConeAngle"] = **m_outerConeAngle;
	udm["coneStartOffset"] = **m_coneStartOffset;
}
void BaseEnvLightSpotComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	udm["blendFraction"](**m_blendFraction);
	udm["outerConeAngle"](**m_outerConeAngle);
	udm["coneStartOffset"](**m_coneStartOffset);
}

void BaseEnvLightSpotComponent::SetOuterConeAngle(umath::Degree ang) {*m_outerConeAngle = ang;}
umath::Degree BaseEnvLightSpotComponent::GetOuterConeAngle() const {return *m_outerConeAngle;}

void BaseEnvLightSpotComponent::SetInnerConeAngle(umath::Degree ang)
{
	SetBlendFraction(CalcBlendFraction(GetOuterConeAngle(),ang));
}
umath::Degree BaseEnvLightSpotComponent::GetInnerConeAngle() const
{
	return CalcInnerConeAngle(GetOuterConeAngle(),GetBlendFraction());
}
umath::Fraction BaseEnvLightSpotComponent::GetBlendFraction() const {return *m_blendFraction;}
void BaseEnvLightSpotComponent::SetBlendFraction(umath::Fraction fraction) {*m_blendFraction = fraction;}

const util::PFloatProperty &BaseEnvLightSpotComponent::GetBlendFractionProperty() const {return m_blendFraction;}
const util::PFloatProperty &BaseEnvLightSpotComponent::GetOuterConeAngleProperty() const {return m_outerConeAngle;}
const util::PFloatProperty &BaseEnvLightSpotComponent::GetConeStartOffsetProperty() const {return m_coneStartOffset;}

void BaseEnvLightSpotComponent::SetConeStartOffset(float offset) {*m_coneStartOffset = offset;}
float BaseEnvLightSpotComponent::GetConeStartOffset() const {return *m_coneStartOffset;}

float BaseEnvLightSpotComponent::CalcConeFalloff(const Vector3 &point) const
{
	return CalcConeFalloff(
		GetEntity().GetPosition(),GetEntity().GetForward(),
		GetOuterConeAngle(),GetInnerConeAngle(),point
	);
}
float BaseEnvLightSpotComponent::CalcDistanceFalloff(const Vector3 &point) const
{
	auto *radiusC = dynamic_cast<pragma::BaseRadiusComponent*>(GetEntity().FindComponent("radius").get());
	if(!radiusC)
		return 0.f;
	return BaseEnvLightComponent::CalcDistanceFalloff(GetEntity().GetPosition(),point,radiusC->GetRadius());
}
