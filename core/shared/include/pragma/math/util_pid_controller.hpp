/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __UTIL_PID_CONTROLLER_HPP__
#define __UTIL_PID_CONTROLLER_HPP__

#include "pragma/networkdefinitions.h"

namespace util
{
	class DLLNETWORK PIDController
	{
	public:
		PIDController()=default;
		PIDController(float p,float i,float d,float min=std::numeric_limits<float>::lowest(),float max=std::numeric_limits<float>::max());
		void SetProportionalTerm(float term);
		void SetIntegralTerm(float term);
		void SetDerivativeTerm(float term);
		void SetTerms(float p,float i,float d);

		float GetProportionalTerm() const;
		float GetIntegralTerm() const;
		float GetDerivativeTerm() const;
		void GetTerms(float &p,float &i,float &d) const;

		void SetRange(float min,float max);
		std::pair<float,float> GetRange() const;
		void ClearRange();
		void SetMin(float min);
		void SetMax(float max);

		float Calculate(float processFeedback,float setPoint,float dt);
		void Reset();
	protected:
		float m_proportional = 0.f;
		float m_integral = 0.f;
		float m_derivative = 0.f;
		std::pair<float,float> m_range = {std::numeric_limits<float>::lowest(),std::numeric_limits<float>::max()};

		float m_previousError = 0.f;
		float m_previousIntegral = 0.f;
	};
};

#endif
