// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.constraints.base_cone_twist;

export import :entities.components.point.constraints.base;

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

		virtual void InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt) override;
	};
};
