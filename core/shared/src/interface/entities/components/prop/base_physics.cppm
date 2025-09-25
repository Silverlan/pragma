// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include <memory>

export module pragma.shared:entities.components.props.base_physics;

export namespace pragma {
	class DLLNETWORK BasePropPhysicsComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	};
};
