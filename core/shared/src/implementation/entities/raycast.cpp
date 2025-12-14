// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.raycast;

pragma::physics::TraceData pragma::util::get_entity_trace_data(ecs::BaseEntity &ent)
{
	auto pCharComponent = ent.GetCharacterComponent();
	if(pCharComponent.valid())
		return pCharComponent->GetAimTraceData();
	auto pTrComponent = ent.GetTransformComponent();
	return pTrComponent != nullptr ? get_entity_trace_data(*pTrComponent) : physics::TraceData {};
}
