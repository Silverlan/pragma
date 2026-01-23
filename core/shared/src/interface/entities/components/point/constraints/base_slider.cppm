// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.constraints.base_slider;

export import :entities.components.point.constraints.base;

export namespace pragma {
	class DLLNETWORK BasePointConstraintSliderComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	  protected:
		float m_kvLimitLinLow = 0.f;
		float m_kvLimitLinHigh = -1.f;
		virtual void InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt) override;
	};
};
