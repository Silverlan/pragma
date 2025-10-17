// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:networking.velocity_correction;

export namespace nwm {
	class DLLNETWORK VelocityCorrection {
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
