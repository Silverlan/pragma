// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.point.constraints.base_ball_socket;

export import :entities.components.point.constraints.base;

export namespace pragma {
	class DLLNETWORK BasePointConstraintBallSocketComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	  protected:
		virtual void InitializeConstraint(pragma::ecs::BaseEntity *src, pragma::ecs::BaseEntity *tgt) override;
	};
};
