/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networking/nwm_velocity_correction.hpp"

void nwm::VelocityCorrection::SetLinearCorrectionVelocity(const Vector3 &v) {m_linearCorrectionVel = v;}
const Vector3 &nwm::VelocityCorrection::GetLinearCorrectionVelocity() const {return m_linearCorrectionVel;}
void nwm::VelocityCorrection::ResetLinearCorrectionVelocity() {SetLinearCorrectionVelocity({});}
void nwm::VelocityCorrection::CorrectLinearVelocity(Vector3 &vel) {vel += GetLinearCorrectionVelocity();}

void nwm::VelocityCorrection::SetAngularCorrectionVelocity(const Vector3 &v) {m_angularCorrectionVel = v;}
const Vector3 &nwm::VelocityCorrection::GetAngularCorrectionVelocity() const {return m_angularCorrectionVel;}
void nwm::VelocityCorrection::ResetAngularCorrectionVelocity() {SetAngularCorrectionVelocity({});}
void nwm::VelocityCorrection::CorrectAngularVelocity(Vector3 &vel) {vel += GetAngularCorrectionVelocity();}
