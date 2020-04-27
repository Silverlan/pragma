/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __ENTITY_PROPERTY_HPP__
#define __ENTITY_PROPERTY_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK EntityProperty
		: public util::SimpleProperty<EntityProperty,EntityHandle>
	{
	public:
		EntityProperty();
		EntityProperty(const EntityHandle &hEnt);
		using util::SimpleProperty<EntityProperty,EntityHandle>::operator=;
	};
	using PEntityProperty = std::shared_ptr<EntityProperty>;
};

#endif
