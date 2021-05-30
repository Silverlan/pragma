/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/baseentity_trace.hpp"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"

TraceData util::get_entity_trace_data(BaseEntity &ent)
{
	auto pCharComponent = ent.GetCharacterComponent();
	if(pCharComponent.valid())
		return pCharComponent->GetAimTraceData();
	auto pTrComponent = ent.GetTransformComponent();
	return pTrComponent != nullptr ? get_entity_trace_data(*pTrComponent) : TraceData{};
}
