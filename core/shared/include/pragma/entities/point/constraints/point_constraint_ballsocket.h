// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __POINT_CONSTRAINT_BALLSOCKET_H__
#define __POINT_CONSTRAINT_BALLSOCKET_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

namespace pragma {
	class DLLNETWORK BasePointConstraintBallSocketComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	  protected:
		virtual void InitializeConstraint(BaseEntity *src, BaseEntity *tgt) override;
	};
};

#endif
