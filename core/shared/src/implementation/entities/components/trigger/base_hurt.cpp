// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.triggers.base_hurt;

using namespace pragma;

void BaseTriggerHurtComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("model");
}
