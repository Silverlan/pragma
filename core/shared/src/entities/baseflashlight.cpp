/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/baseflashlight.h"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include <mathutil/umath_lighting.hpp>

using namespace pragma;

void BaseFlashlightComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto *pLightComponent = static_cast<pragma::BaseEnvLightComponent*>(ent.AddComponent("light").get());
	auto *pLightSpotComponent = static_cast<pragma::BaseEnvLightSpotComponent*>(ent.AddComponent("light_spot").get());
	ent.AddComponent("sound_emitter");
	auto *pRadiusComponent = static_cast<pragma::BaseRadiusComponent*>(ent.AddComponent("radius").get());
	auto *pColorComponent = static_cast<pragma::BaseColorComponent*>(ent.AddComponent("color").get());

	if(pLightSpotComponent != nullptr)
	{
		pLightSpotComponent->SetBlendFraction(0.4f);
		pLightSpotComponent->SetOuterConeAngle(50.f);
	}
	if(pRadiusComponent != nullptr)
		pRadiusComponent->SetRadius(512.f);
	if(pColorComponent != nullptr)
		pColorComponent->SetColor(ulighting::color_temperature_to_color(3'500));
	if(pLightComponent)
		pLightComponent->SetLightIntensity(10.f,BaseEnvLightComponent::LightIntensityType::Lumen);
}
