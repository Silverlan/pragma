// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

export module pragma.shared:entities.components.point.constraints.base_slider;

export namespace pragma {
	class DLLNETWORK BasePointConstraintSliderComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	  protected:
		float m_kvLimitLinLow = 0.f;
		float m_kvLimitLinHigh = -1.f;
		virtual void InitializeConstraint(BaseEntity *src, BaseEntity *tgt) override;
	};
};
