/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __POINT_CONSTRAINT_CONETWIST_H__
#define __POINT_CONSTRAINT_CONETWIST_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

namespace pragma
{
	class DLLNETWORK BasePointConstraintConeTwistComponent
		: public BasePointConstraintComponent
	{
	public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	protected:
		float m_kvSwingSpan1 = 0.f;
		float m_kvSwingSpan2 = 0.f;
		float m_kvTwistSpan = 0.f;
		float m_kvSoftness = 1.f;
		float m_kvRelaxationFactor = 1.f;

		virtual void InitializeConstraint(BaseEntity *src,BaseEntity *tgt) override;
	};
};

#endif
