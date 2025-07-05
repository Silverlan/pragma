// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
	auto *pLightComponent = static_cast<pragma::BaseEnvLightComponent *>(ent.AddComponent("light").get());
	auto *pLightSpotComponent = static_cast<pragma::BaseEnvLightSpotComponent *>(ent.AddComponent("light_spot").get());
	ent.AddComponent("sound_emitter");
	auto *pRadiusComponent = static_cast<pragma::BaseRadiusComponent *>(ent.AddComponent("radius").get());
	auto *pColorComponent = static_cast<pragma::BaseColorComponent *>(ent.AddComponent("color").get());

	if(pLightSpotComponent != nullptr) {
		pLightSpotComponent->SetBlendFraction(0.4f);
		pLightSpotComponent->SetOuterConeAngle(50.f);
	}
	if(pRadiusComponent != nullptr)
		pRadiusComponent->SetRadius(512.f);
	if(pColorComponent != nullptr)
		pColorComponent->SetColor(ulighting::color_temperature_to_color(3'500));
	if(pLightComponent)
		pLightComponent->SetLightIntensity(10.f, BaseEnvLightComponent::LightIntensityType::Lumen);
}
