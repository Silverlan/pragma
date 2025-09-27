// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/logic/logic_relay.h"

module pragma.shared;

import :entities.components.logic.base_relay;

using namespace pragma;

void BaseLogicRelayComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("toggle");
}
