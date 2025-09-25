// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

export module pragma.shared:entities.components.point.constraints.base_cone_twist;

export namespace pragma {
	class DLLNETWORK BasePointConstraintConeTwistComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	  protected:
		float m_kvSwingSpan1 = 0.f;
		float m_kvSwingSpan2 = 0.f;
		float m_kvTwistSpan = 0.f;
		float m_kvSoftness = 1.f;
		float m_kvRelaxationFactor = 1.f;

		virtual void InitializeConstraint(BaseEntity *src, BaseEntity *tgt) override;
	};
};
