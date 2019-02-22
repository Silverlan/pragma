#include "stdafx_shared.h"
#include "pragma/entities/environment/env_camera.h"
#include <algorithm>

using namespace pragma;

void BaseEnvCameraComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("toggle");
}
