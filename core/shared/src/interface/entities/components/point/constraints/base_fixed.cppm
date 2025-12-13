// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.constraints.base_fixed;

export import :entities.components.point.constraints.base;

export namespace pragma {
	class DLLNETWORK BasePointConstraintFixedComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
	  protected:
		virtual void InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt) override;
	};
};
