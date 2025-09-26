// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/physics/raytraces.h"

export module pragma.shared:entities.raycast;

export namespace util {
	DLLNETWORK TraceData get_entity_trace_data(BaseEntity &ent);
	DLLNETWORK TraceData get_entity_trace_data(::pragma::BaseTransformComponent &component);
};
