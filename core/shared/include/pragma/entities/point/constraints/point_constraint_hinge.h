/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __POINT_CONSTRAINT_HINGE_H__
#define __POINT_CONSTRAINT_HINGE_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

namespace pragma
{
	class DLLNETWORK BasePointConstraintHingeComponent
		: public BasePointConstraintComponent
	{
	public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	protected:
		float m_kvLimitLow = 0.f;
		float m_kvLimitHigh = -1.f;
		float m_kvLimitSoftness = 0.9f;
		float m_kvLimitBiasFactor = 0.3f;
		float m_kvLimitRelaxationFactor = 1.f;
		virtual void InitializeConstraint(BaseEntity *src,BaseEntity *tgt) override;
	};
};

#endif
