/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/math/util_pid_controller.hpp"

util::PIDController::PIDController(float p,float i,float d,float min,float max)
	: m_proportional(p),m_integral(i),m_derivative(d),
	m_range(min,max)
{}

void util::PIDController::SetProportionalTerm(float term) {m_proportional = term;}
void util::PIDController::SetIntegralTerm(float term) {m_integral = term;}
void util::PIDController::SetDerivativeTerm(float term) {m_derivative = term;}
void util::PIDController::SetTerms(float p,float i,float d)
{
	SetProportionalTerm(p);
	SetIntegralTerm(i);
	SetDerivativeTerm(d);
}

float util::PIDController::GetProportionalTerm() const {return m_proportional;}
float util::PIDController::GetIntegralTerm() const {return m_integral;}
float util::PIDController::GetDerivativeTerm() const {return m_derivative;}
void util::PIDController::GetTerms(float &p,float &i,float &d) const
{
	p = GetProportionalTerm();
	i = GetIntegralTerm();
	d = GetDerivativeTerm();
}

void util::PIDController::SetRange(float min,float max) {m_range = {min,max};}
std::pair<float,float> util::PIDController::GetRange() const {return m_range;}
void util::PIDController::ClearRange() {SetRange(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::max());}
void util::PIDController::SetMin(float min) {m_range.first = min;}
void util::PIDController::SetMax(float max) {m_range.second = max;}

void util::PIDController::Reset()
{
	m_previousError = 0.f;
	m_previousIntegral = 0.f;
}

float util::PIDController::Calculate(float processFeedback,float setPoint,float dt)
{
	auto kp = m_proportional;
	auto ki = m_integral;
	auto kd = m_derivative;

	auto err = setPoint -processFeedback;
	auto prevErr = m_previousError;
	auto prevIntegral = m_previousIntegral;
	auto integral = prevIntegral +(err *dt);
	integral = umath::clamp(integral,m_range.first,m_range.second);
	auto derivative = (err -prevErr) /dt;
	auto output = (kp *err) +(ki *integral) +(kd *derivative);
	output = umath::clamp(output,m_range.first,m_range.second);

	m_previousIntegral = integral;
	m_previousError = err;
	return output;
}
