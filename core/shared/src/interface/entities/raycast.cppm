// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.raycast;

export import :physics.raycast;

export {
	namespace pragma {
		class BaseTransformComponent;
	};
	namespace pragma::ecs {class BaseEntity;}
	namespace util {
		DLLNETWORK TraceData get_entity_trace_data(pragma::ecs::BaseEntity &ent);
		DLLNETWORK TraceData get_entity_trace_data(::pragma::BaseTransformComponent &component);
	};
};
