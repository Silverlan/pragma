// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.point.constraints.base_dof;

export import :entities.components.point.constraints.base;

export namespace pragma {
	class DLLNETWORK BasePointConstraintDoFComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	  protected:
		Vector3 m_kvLimLinLower;
		Vector3 m_kvLimLinUpper;
		Vector3 m_kvLimAngLower;
		Vector3 m_kvLimAngUpper;
		virtual void InitializeConstraint(ecs::BaseEntity *src, ecs::BaseEntity *tgt) override;
	};
};
