// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>

export module pragma.shared:entities.components.logic.base_relay;

export namespace pragma {
	class DLLNETWORK BaseLogicRelayComponent : public pragma::BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};
