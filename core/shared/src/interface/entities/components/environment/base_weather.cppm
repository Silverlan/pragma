// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"

export module pragma.shared:entities.components.environment.base_weather;

export namespace pragma {
	class DLLNETWORK BaseEnvWeatherComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
	};
};
