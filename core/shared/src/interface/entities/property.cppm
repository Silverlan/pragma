// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.property;

export import :entities.base_entity_handle;
export import pragma.util;

export namespace pragma {
	class DLLNETWORK EntityProperty : public util::SimpleProperty<EntityProperty, EntityHandle> {
	  public:
		EntityProperty();
		EntityProperty(const EntityHandle &hEnt);
	};
	using PEntityProperty = std::shared_ptr<EntityProperty>;
};
