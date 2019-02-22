#ifndef __POINT_CONSTRAINT_SLIDER_H__
#define __POINT_CONSTRAINT_SLIDER_H__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/point/constraints/point_constraint_base.h"

namespace pragma
{
	class DLLNETWORK BasePointConstraintSliderComponent
		: public BasePointConstraintComponent
	{
	public:
		using BasePointConstraintComponent::BasePointConstraintComponent;
		virtual void Initialize() override;
	protected:
		float m_kvLimitLinLow = 0.f;
		float m_kvLimitLinHigh = -1.f;
		virtual void InitializeConstraint(BaseEntity *src,BaseEntity *tgt) override;
	};
};

#endif