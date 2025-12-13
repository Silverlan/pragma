// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.constraints.base_hinge;

export import :entities.components.point.constraints.base;

export namespace pragma {
	class DLLNETWORK BasePointConstraintHingeComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	  protected:
		float m_kvLimitLow = 0.f;
		float m_kvLimitHigh = -1.f;
		float m_kvLimitSoftness = 0.9f;
		float m_kvLimitBiasFactor = 0.3f;
		float m_kvLimitRelaxationFactor = 1.f;
		virtual void InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt) override;
	};
};
