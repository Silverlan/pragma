// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.raycast;

export import :physics.raycast;

export {
	namespace pragma {
		class BaseTransformComponent;
	};
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma::util {
		DLLNETWORK physics::TraceData get_entity_trace_data(ecs::BaseEntity &ent);
		DLLNETWORK physics::TraceData get_entity_trace_data(BaseTransformComponent &component);
	};
};
