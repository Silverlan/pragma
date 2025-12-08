// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :networking.velocity_correction;

void pragma::networking::VelocityCorrection::SetLinearCorrectionVelocity(const Vector3 &v) { m_linearCorrectionVel = v; }
const Vector3 &pragma::networking::VelocityCorrection::GetLinearCorrectionVelocity() const { return m_linearCorrectionVel; }
void pragma::networking::VelocityCorrection::ResetLinearCorrectionVelocity() { SetLinearCorrectionVelocity({}); }
void pragma::networking::VelocityCorrection::CorrectLinearVelocity(Vector3 &vel) { vel += GetLinearCorrectionVelocity(); }

void pragma::networking::VelocityCorrection::SetAngularCorrectionVelocity(const Vector3 &v) { m_angularCorrectionVel = v; }
const Vector3 &pragma::networking::VelocityCorrection::GetAngularCorrectionVelocity() const { return m_angularCorrectionVel; }
void pragma::networking::VelocityCorrection::ResetAngularCorrectionVelocity() { SetAngularCorrectionVelocity({}); }
void pragma::networking::VelocityCorrection::CorrectAngularVelocity(Vector3 &vel) { vel += GetAngularCorrectionVelocity(); }
