/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __POINT_CONSTRAINT_BALLSOCKET_H__
#define __POINT_CONSTRAINT_BALLSOCKET_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

namespace pragma
{
	class DLLNETWORK BasePointConstraintBallSocketComponent
		: public BasePointConstraintComponent
	{
	public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	protected:
		virtual void InitializeConstraint(BaseEntity *src,BaseEntity *tgt) override;
	};
};

#endif
