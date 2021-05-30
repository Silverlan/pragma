/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_GENERIC_COMPONENT_HPP__
#define __BASE_GENERIC_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK BaseGenericComponent
		: public BaseEntityComponent
	{
	protected:
		BaseGenericComponent(BaseEntity &ent);
	};
};

#endif
