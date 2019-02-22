#include "stdafx_shared.h"
#include "pragma/entities/baseflashlight.h"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"

using namespace pragma;

void BaseFlashlightComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("light");
	auto *pLightSpotComponent = static_cast<pragma::BaseEnvLightSpotComponent*>(ent.AddComponent("light_spot").get());
	ent.AddComponent("sound_emitter");
	auto *pRadiusComponent = static_cast<pragma::BaseRadiusComponent*>(ent.AddComponent("radius").get());
	auto *pColorComponent = static_cast<pragma::BaseColorComponent*>(ent.AddComponent("color").get());

	if(pLightSpotComponent != nullptr)
	{
		pLightSpotComponent->SetInnerCutoffAngle(15.f);
		pLightSpotComponent->SetOuterCutoffAngle(25.f);
	}
	if(pRadiusComponent != nullptr)
		pRadiusComponent->SetRadius(512.f);
	if(pColorComponent != nullptr)
		pColorComponent->SetColor(Color(355,355,153,6'000));
}
