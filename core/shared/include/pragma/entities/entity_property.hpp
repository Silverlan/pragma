// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENTITY_PROPERTY_HPP__
#define __ENTITY_PROPERTY_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <sharedutils/property/util_property.hpp>

namespace pragma {
	class DLLNETWORK EntityProperty : public util::SimpleProperty<EntityProperty, EntityHandle> {
	  public:
		EntityProperty();
		EntityProperty(const EntityHandle &hEnt);
		using util::SimpleProperty<EntityProperty, EntityHandle>::operator=;
	};
	using PEntityProperty = std::shared_ptr<EntityProperty>;
};

#endif
