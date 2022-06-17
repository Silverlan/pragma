/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/entities/components/base_radius_component.hpp"
#include <sharedutils/util_pragma.hpp>
#include <algorithm>

using namespace pragma;

Candela BaseEnvLightPointComponent::CalcIntensityAtPoint(
	const Vector3 &lightPos,float radius,Candela intensity,const Vector3 &point
)
{
	return intensity *BaseEnvLightComponent::CalcDistanceFalloff(lightPos,point,radius);
}
void BaseEnvLightPointComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("light");
	ent.AddComponent("radius");
}
util::EventReply BaseEnvLightPointComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
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
				GetEntity().GetPosition(),radius,cLight->GetLightIntensityCandela(),levData.pos
			);
		}
		return util::EventReply::Handled;
	}
	return BaseEntityComponent::HandleEvent(eventId,evData);
}
float BaseEnvLightPointComponent::CalcDistanceFalloff(const Vector3 &point) const
{
	auto *radiusC = dynamic_cast<pragma::BaseRadiusComponent*>(GetEntity().FindComponent("radius").get());
	if(!radiusC)
		return 0.f;
	return BaseEnvLightComponent::CalcDistanceFalloff(GetEntity().GetPosition(),point,radiusC->GetRadius());
}
