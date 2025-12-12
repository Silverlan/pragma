// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :math.easing;

// Source: http://robertpenner.com/easing/
namespace ease {
	namespace back {
		// Overshooting cubic easing;
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace bounce {
		// Exponentially-decaying bounce easing
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace circular {
		// Circular easing; sqrt(1 - p^2)
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace cubic {
		// Cubic easing; p^3
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace elastic {
		// Exponentially-damped sine wave easing
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace exponential {
		// Exponential easing, base 2
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace linear {
		// Linear interpolation (no easing)
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace quadratic {
		// Quadratic easing; p^2
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace quartic {
		// Quartic easing; p^4
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace quintic {
		// Quintic easing; p^5
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
	namespace sine {
		// Sine wave easing; sin(p * PI/2)
		static float ease_in(float p);
		static float ease_out(float p);
		static float ease_in_out(float p);
	};
};

float ease::back::ease_in(float p) { return p * p * p - p * sin(p * pragma::math::pi); }
float ease::back::ease_out(float p)
{
	float f = (1 - p);
	return 1 - (f * f * f - f * sin(f * pragma::math::pi));
}
float ease::back::ease_in_out(float p)
{
	if(p < 0.5) {
		float f = 2 * p;
		return 0.5 * (f * f * f - f * sin(f * pragma::math::pi));
	}
	else {
		float f = (1 - (2 * p - 1));
		return 0.5 * (1 - (f * f * f - f * sin(f * pragma::math::pi))) + 0.5;
	}
}

float ease::bounce::ease_in(float p) { return 1 - ease_out(1 - p); }
float ease::bounce::ease_out(float p)
{
	if(p < 4 / 11.0)
		return (121 * p * p) / 16.0;
	else if(p < 8 / 11.0)
		return (363 / 40.0 * p * p) - (99 / 10.0 * p) + 17 / 5.0;
	else if(p < 9 / 10.0)
		return (4356 / 361.0 * p * p) - (35442 / 1805.0 * p) + 16061 / 1805.0;
	else
		return (54 / 5.0 * p * p) - (513 / 25.0 * p) + 268 / 25.0;
}
float ease::bounce::ease_in_out(float p)
{
	if(p < 0.5)
		return 0.5 * ease_in(p * 2);
	else
		return 0.5 * ease_out(p * 2 - 1) + 0.5;
}

float ease::circular::ease_in(float p) { return 1 - sqrt(1 - (p * p)); }
float ease::circular::ease_out(float p) { return sqrt((2 - p) * p); }
float ease::circular::ease_in_out(float p)
{
	if(p < 0.5)
		return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
	else
		return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
}

float ease::cubic::ease_in(float p) { return p * p * p; }
float ease::cubic::ease_out(float p)
{
	float f = (p - 1);
	return f * f * f + 1;
}
float ease::cubic::ease_in_out(float p)
{
	if(p < 0.5)
		return 4 * p * p * p;
	else {
		float f = ((2 * p) - 2);
		return 0.5 * f * f * f + 1;
	}
}

float ease::elastic::ease_in(float p) { return sin(13 * pragma::math::pi_2 * p) * pow(2, 10 * (p - 1)); }
float ease::elastic::ease_out(float p) { return sin(-13 * pragma::math::pi_2 * (p + 1)) * pow(2, -10 * p) + 1; }
float ease::elastic::ease_in_out(float p)
{
	if(p < 0.5)
		return 0.5 * sin(13 * pragma::math::pi_2 * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
	else
		return 0.5 * (sin(-13 * pragma::math::pi_2 * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
}

float ease::exponential::ease_in(float p) { return (p == 0.0) ? p : pow(2, 10 * (p - 1)); }
float ease::exponential::ease_out(float p) { return (p == 1.0) ? p : 1 - pow(2, -10 * p); }
float ease::exponential::ease_in_out(float p)
{
	if(p == 0.0 || p == 1.0)
		return p;

	if(p < 0.5)
		return 0.5 * pow(2, (20 * p) - 10);
	else
		return -0.5 * pow(2, (-20 * p) + 10) + 1;
}

float ease::linear::ease_in(float p) { return p; }
float ease::linear::ease_out(float p) { return p; }
float ease::linear::ease_in_out(float p) { return p; }

float ease::quadratic::ease_in(float p) { return p * p; }
float ease::quadratic::ease_out(float p) { return -(p * (p - 2)); }
float ease::quadratic::ease_in_out(float p)
{
	if(p < 0.5)
		return 2 * p * p;
	else
		return (-2 * p * p) + (4 * p) - 1;
}

float ease::quartic::ease_in(float p) { return p * p * p * p; }
float ease::quartic::ease_out(float p)
{
	float f = (p - 1);
	return f * f * f * (1 - p) + 1;
}
float ease::quartic::ease_in_out(float p)
{
	if(p < 0.5)
		return 8 * p * p * p * p;
	else {
		float f = (p - 1);
		return -8 * f * f * f * f + 1;
	}
}

float ease::quintic::ease_in(float p) { return p * p * p * p * p; }
float ease::quintic::ease_out(float p)
{
	float f = (p - 1);
	return f * f * f * f * f + 1;
}
float ease::quintic::ease_in_out(float p)
{
	if(p < 0.5)
		return 16 * p * p * p * p * p;
	else {
		float f = ((2 * p) - 2);
		return 0.5 * f * f * f * f * f + 1;
	}
}

float ease::sine::ease_in(float p) { return sin((p - 1) * pragma::math::pi_2) + 1; }
float ease::sine::ease_out(float p) { return sin(p * pragma::math::pi_2); }
float ease::sine::ease_in_out(float p) { return 0.5 * (1 - cos(p * pragma::math::pi)); }

float pragma::math::ease_in(float t, EaseType type)
{
	switch(type) {
	case EaseType::Back:
		return ease::back::ease_in(t);
	case EaseType::Bounce:
		return ease::bounce::ease_in(t);
	case EaseType::Circular:
		return ease::circular::ease_in(t);
	case EaseType::Cubic:
		return ease::cubic::ease_in(t);
	case EaseType::Elastic:
		return ease::elastic::ease_in(t);
	case EaseType::Exponential:
		return ease::exponential::ease_in(t);
	case EaseType::Linear:
		return ease::linear::ease_in(t);
	case EaseType::Quadratic:
		return ease::quadratic::ease_in(t);
	case EaseType::Quartic:
		return ease::quartic::ease_in(t);
	case EaseType::Quintic:
		return ease::quintic::ease_in(t);
	case EaseType::Sine:
		return ease::sine::ease_in(t);
	}
	return 0.f;
}
float pragma::math::ease_out(float t, EaseType type)
{
	switch(type) {
	case EaseType::Back:
		return ease::back::ease_out(t);
	case EaseType::Bounce:
		return ease::bounce::ease_out(t);
	case EaseType::Circular:
		return ease::circular::ease_out(t);
	case EaseType::Cubic:
		return ease::cubic::ease_out(t);
	case EaseType::Elastic:
		return ease::elastic::ease_out(t);
	case EaseType::Exponential:
		return ease::exponential::ease_out(t);
	case EaseType::Linear:
		return ease::linear::ease_out(t);
	case EaseType::Quadratic:
		return ease::quadratic::ease_out(t);
	case EaseType::Quartic:
		return ease::quartic::ease_out(t);
	case EaseType::Quintic:
		return ease::quintic::ease_out(t);
	case EaseType::Sine:
		return ease::sine::ease_out(t);
	}
	return 0.f;
}
float pragma::math::ease_in_out(float t, EaseType type)
{
	switch(type) {
	case EaseType::Back:
		return ease::back::ease_in_out(t);
	case EaseType::Bounce:
		return ease::bounce::ease_in_out(t);
	case EaseType::Circular:
		return ease::circular::ease_in_out(t);
	case EaseType::Cubic:
		return ease::cubic::ease_in_out(t);
	case EaseType::Elastic:
		return ease::elastic::ease_in_out(t);
	case EaseType::Exponential:
		return ease::exponential::ease_in_out(t);
	case EaseType::Linear:
		return ease::linear::ease_in_out(t);
	case EaseType::Quadratic:
		return ease::quadratic::ease_in_out(t);
	case EaseType::Quartic:
		return ease::quartic::ease_in_out(t);
	case EaseType::Quintic:
		return ease::quintic::ease_in_out(t);
	case EaseType::Sine:
		return ease::sine::ease_in_out(t);
	}
	return 0.f;
}
