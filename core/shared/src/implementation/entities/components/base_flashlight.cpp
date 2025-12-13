// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_flashlight;

using namespace pragma;

void BaseFlashlightComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto *pLightComponent = static_cast<BaseEnvLightComponent *>(ent.AddComponent("light").get());
	auto *pLightSpotComponent = static_cast<BaseEnvLightSpotComponent *>(ent.AddComponent("light_spot").get());
	ent.AddComponent("sound_emitter");
	auto *pRadiusComponent = static_cast<BaseRadiusComponent *>(ent.AddComponent("radius").get());
	auto *pColorComponent = static_cast<BaseColorComponent *>(ent.AddComponent("color").get());

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
