#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include <algorithm>

using namespace pragma;

void BaseEnvLightPointComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("light");
	ent.AddComponent("radius");
}
