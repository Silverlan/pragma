// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:math.pid_controller;

export import std;

export {
	namespace pragma::math {
		class DLLNETWORK PIDController {
		  public:
			PIDController() = default;
			PIDController(float p, float i, float d, float min = std::numeric_limits<float>::lowest(), float max = std::numeric_limits<float>::max());
			void SetProportionalTerm(float term);
			void SetIntegralTerm(float term);
			void SetDerivativeTerm(float term);
			void SetTerms(float p, float i, float d);

			float GetProportionalTerm() const;
			float GetIntegralTerm() const;
			float GetDerivativeTerm() const;
			void GetTerms(float &p, float &i, float &d) const;

			void SetRange(float min, float max);
			std::pair<float, float> GetRange() const;
			void ClearRange();
			void SetMin(float min);
			void SetMax(float max);

			float Calculate(float processFeedback, float setPoint, float dt);
			void Reset();
		  protected:
			float m_proportional = 0.f;
			float m_integral = 0.f;
			float m_derivative = 0.f;
			std::pair<float, float> m_range = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};

			float m_previousError = 0.f;
			float m_previousIntegral = 0.f;
		};
	};

	DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::math::PIDController &o);
};
