// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include <sharedutils/property/util_property.hpp>

export module pragma.shared:entities.property;

export namespace pragma {
	class DLLNETWORK EntityProperty : public util::SimpleProperty<EntityProperty, EntityHandle> {
	  public:
		EntityProperty();
		EntityProperty(const EntityHandle &hEnt);
		using util::SimpleProperty<EntityProperty, EntityHandle>::operator=;
	};
	using PEntityProperty = std::shared_ptr<EntityProperty>;
};
