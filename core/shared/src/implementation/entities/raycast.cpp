// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/entities/baseentity_trace.hpp"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"

module pragma.shared;

import :entities.raycast;

TraceData util::get_entity_trace_data(BaseEntity &ent)
{
	auto pCharComponent = ent.GetCharacterComponent();
	if(pCharComponent.valid())
		return pCharComponent->GetAimTraceData();
	auto pTrComponent = ent.GetTransformComponent();
	return pTrComponent != nullptr ? get_entity_trace_data(*pTrComponent) : TraceData {};
}
