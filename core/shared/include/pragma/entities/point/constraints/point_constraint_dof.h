/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __POINT_CONSTRAINT_DOF_H__
#define __POINT_CONSTRAINT_DOF_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

namespace pragma {
	class DLLNETWORK BasePointConstraintDoFComponent : public BasePointConstraintComponent {
	  public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	  protected:
		Vector3 m_kvLimLinLower;
		Vector3 m_kvLimLinUpper;
		Vector3 m_kvLimAngLower;
		Vector3 m_kvLimAngUpper;
		virtual void InitializeConstraint(BaseEntity *src, BaseEntity *tgt) override;
	};
};

#endif
