#include "stdafx_shared.h"
#include "pragma/entities/trigger/base_trigger_hurt.hpp"

using namespace pragma;

void BaseTriggerHurtComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("model");
}
