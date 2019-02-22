#include "stdafx_shared.h"
#include "pragma/entities/trigger/base_trigger_remove.h"

using namespace pragma;

void BaseTriggerRemoveComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("model");
}
