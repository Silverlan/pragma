#include "stdafx_shared.h"
#include "pragma/entities/info/info_landmark.hpp"

using namespace pragma;

void BaseInfoLandmarkComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("name");
	ent.AddComponent("transform");
}
