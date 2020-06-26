/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASEENTITY_TRACE_HPP__
#define __BASEENTITY_TRACE_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/raytraces.h"

class BaseEntity;
namespace pragma
{
	class BaseTransformComponent;
};
namespace util
{
	DLLNETWORK TraceData get_entity_trace_data(BaseEntity &ent);
	DLLNETWORK TraceData get_entity_trace_data(::pragma::BaseTransformComponent &component);
};

#endif
