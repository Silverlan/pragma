/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __NWM_VELOCITY_CORRECTION_HPP__
#define __NWM_VELOCITY_CORRECTION_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>

namespace nwm
{
	class DLLNETWORK VelocityCorrection
	{
	public:
		void SetLinearCorrectionVelocity(const Vector3 &v);
		const Vector3 &GetLinearCorrectionVelocity() const;
		void CorrectLinearVelocity(Vector3 &vel);
		void ResetLinearCorrectionVelocity();

		void SetAngularCorrectionVelocity(const Vector3 &v);
		const Vector3 &GetAngularCorrectionVelocity() const;
		void CorrectAngularVelocity(Vector3 &vel);
		void ResetAngularCorrectionVelocity();
	private:
		Vector3 m_linearCorrectionVel = {};
		Vector3 m_angularCorrectionVel = {};
	};
};

#endif
