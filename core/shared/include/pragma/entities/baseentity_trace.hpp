// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASEENTITY_TRACE_HPP__
#define __BASEENTITY_TRACE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/raytraces.h"

class BaseEntity;
namespace pragma {
	class BaseTransformComponent;
};
namespace util {
	DLLNETWORK TraceData get_entity_trace_data(BaseEntity &ent);
	DLLNETWORK TraceData get_entity_trace_data(::pragma::BaseTransformComponent &component);
};

#endif
