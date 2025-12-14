// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.pid_controller;

pragma::math::PIDController::PIDController(float p, float i, float d, float min, float max) : m_proportional(p), m_integral(i), m_derivative(d), m_range(min, max) {}

void pragma::math::PIDController::SetProportionalTerm(float term) { m_proportional = term; }
void pragma::math::PIDController::SetIntegralTerm(float term) { m_integral = term; }
void pragma::math::PIDController::SetDerivativeTerm(float term) { m_derivative = term; }
void pragma::math::PIDController::SetTerms(float p, float i, float d)
{
	SetProportionalTerm(p);
	SetIntegralTerm(i);
	SetDerivativeTerm(d);
}

float pragma::math::PIDController::GetProportionalTerm() const { return m_proportional; }
float pragma::math::PIDController::GetIntegralTerm() const { return m_integral; }
float pragma::math::PIDController::GetDerivativeTerm() const { return m_derivative; }
void pragma::math::PIDController::GetTerms(float &p, float &i, float &d) const
{
	p = GetProportionalTerm();
	i = GetIntegralTerm();
	d = GetDerivativeTerm();
}

void pragma::math::PIDController::SetRange(float min, float max) { m_range = {min, max}; }
std::pair<float, float> pragma::math::PIDController::GetRange() const { return m_range; }
void pragma::math::PIDController::ClearRange() { SetRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()); }
void pragma::math::PIDController::SetMin(float min) { m_range.first = min; }
void pragma::math::PIDController::SetMax(float max) { m_range.second = max; }

void pragma::math::PIDController::Reset()
{
	m_previousError = 0.f;
	m_previousIntegral = 0.f;
}

float pragma::math::PIDController::Calculate(float processFeedback, float setPoint, float dt)
{
	auto kp = m_proportional;
	auto ki = m_integral;
	auto kd = m_derivative;

	auto err = setPoint - processFeedback;
	auto prevErr = m_previousError;
	auto prevIntegral = m_previousIntegral;
	auto integral = prevIntegral + (err * dt);
	integral = clamp(integral, m_range.first, m_range.second);
	auto derivative = (err - prevErr) / dt;
	auto output = (kp * err) + (ki * integral) + (kd * derivative);
	output = clamp(output, m_range.first, m_range.second);

	m_previousIntegral = integral;
	m_previousError = err;
	return output;
}

std::ostream &operator<<(std::ostream &out, const pragma::math::PIDController &o)
{
	out << "PIDController";
	out << "[PropTerm:" << o.GetProportionalTerm() << "]";
	out << "[IntTerm:" << o.GetIntegralTerm() << "]";
	out << "[DerivTerm:" << o.GetDerivativeTerm() << "]";
	auto range = o.GetRange();
	out << "[Range:" << range.first << "," << range.second << "]";
	return out;
}
