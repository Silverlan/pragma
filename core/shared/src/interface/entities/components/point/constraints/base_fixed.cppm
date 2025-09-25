// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

export module pragma.shared:entities.components.point.constraints.base_fixed;

export namespace pragma {
	class DLLNETWORK BasePointConstraintFixedComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
	  protected:
		virtual void InitializeConstraint(BaseEntity *src, BaseEntity *tgt) override;
	};
};
