// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/networking/nwm_velocity_correction.hpp"

void nwm::VelocityCorrection::SetLinearCorrectionVelocity(const Vector3 &v) { m_linearCorrectionVel = v; }
const Vector3 &nwm::VelocityCorrection::GetLinearCorrectionVelocity() const { return m_linearCorrectionVel; }
void nwm::VelocityCorrection::ResetLinearCorrectionVelocity() { SetLinearCorrectionVelocity({}); }
void nwm::VelocityCorrection::CorrectLinearVelocity(Vector3 &vel) { vel += GetLinearCorrectionVelocity(); }

void nwm::VelocityCorrection::SetAngularCorrectionVelocity(const Vector3 &v) { m_angularCorrectionVel = v; }
const Vector3 &nwm::VelocityCorrection::GetAngularCorrectionVelocity() const { return m_angularCorrectionVel; }
void nwm::VelocityCorrection::ResetAngularCorrectionVelocity() { SetAngularCorrectionVelocity({}); }
void nwm::VelocityCorrection::CorrectAngularVelocity(Vector3 &vel) { vel += GetAngularCorrectionVelocity(); }
