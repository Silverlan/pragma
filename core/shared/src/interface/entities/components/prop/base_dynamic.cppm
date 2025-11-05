// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.props.base_dynamic;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BasePropDynamicComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	};
};
