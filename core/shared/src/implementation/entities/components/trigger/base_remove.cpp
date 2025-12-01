// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.triggers.base_remove;

using namespace pragma;

void BaseTriggerRemoveComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("model");
}
