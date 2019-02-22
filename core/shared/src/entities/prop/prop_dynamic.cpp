#include "stdafx_shared.h"
#include "pragma/entities/prop/prop_dynamic.hpp"

using namespace pragma;

void BasePropDynamicComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("prop");
}

void BasePropDynamicComponent::OnEntitySpawn() {BaseEntityComponent::OnEntitySpawn();}
