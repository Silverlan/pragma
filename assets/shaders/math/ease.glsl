/*
MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Source: https://github.com/glslify/glsl-easings

#include "/math/math.glsl"

float ease_back_in_out(float t)
{
	float f = t < 0.5 ? 2.0 * t : 1.0 - (2.0 * t - 1.0);

	float g = pow(f, 3.0) - f * sin(f * M_PI);

	return t < 0.5 ? 0.5 * g : 0.5 * (1.0 - g) + 0.5;
}

float ease_back_in(float t) { return pow(t, 3.0) - t * sin(t * M_PI); }

float ease_back_out(float t)
{
	float f = 1.0 - t;
	return 1.0 - (pow(f, 3.0) - f * sin(f * M_PI));
}

float ease_bounce_out(float t)
{
	const float a = 4.0 / 11.0;
	const float b = 8.0 / 11.0;
	const float c = 9.0 / 10.0;

	const float ca = 4356.0 / 361.0;
	const float cb = 35442.0 / 1805.0;
	const float cc = 16061.0 / 1805.0;

	float t2 = t * t;

	return t < a ? 7.5625 * t2 : t < b ? 9.075 * t2 - 9.9 * t + 3.4 : t < c ? ca * t2 - cb * t + cc : 10.8 * t * t - 20.52 * t + 10.72;
}

float ease_bounce_in_out(float t) { return t < 0.5 ? 0.5 * (1.0 - ease_bounce_out(1.0 - t * 2.0)) : 0.5 * ease_bounce_out(t * 2.0 - 1.0) + 0.5; }

float ease_bounce_in(float t) { return 1.0 - ease_bounce_out(1.0 - t); }

float ease_circular_in_out(float t) { return t < 0.5 ? 0.5 * (1.0 - sqrt(1.0 - 4.0 * t * t)) : 0.5 * (sqrt((3.0 - 2.0 * t) * (2.0 * t - 1.0)) + 1.0); }

float ease_circular_in(float t) { return 1.0 - sqrt(1.0 - t * t); }

float ease_circular_out(float t) { return sqrt((2.0 - t) * t); }

float ease_cubic_in_out(float t) { return t < 0.5 ? 4.0 * t * t * t : 0.5 * pow(2.0 * t - 2.0, 3.0) + 1.0; }

float ease_cubic_in(float t) { return t * t * t; }

float ease_cubic_out(float t)
{
	float f = t - 1.0;
	return f * f * f + 1.0;
}

float ease_elastic_in_out(float t) { return t < 0.5 ? 0.5 * sin(+13.0 * M_PI_2 * 2.0 * t) * pow(2.0, 10.0 * (2.0 * t - 1.0)) : 0.5 * sin(-13.0 * M_PI_2 * ((2.0 * t - 1.0) + 1.0)) * pow(2.0, -10.0 * (2.0 * t - 1.0)) + 1.0; }

float ease_elastic_in(float t) { return sin(13.0 * t * M_PI_2) * pow(2.0, 10.0 * (t - 1.0)); }

float ease_elastic_out(float t) { return sin(-13.0 * (t + 1.0) * M_PI_2) * pow(2.0, -10.0 * t) + 1.0; }

float ease_exponential_in_out(float t) { return t == 0.0 || t == 1.0 ? t : t < 0.5 ? +0.5 * pow(2.0, (20.0 * t) - 10.0) : -0.5 * pow(2.0, 10.0 - (t * 20.0)) + 1.0; }

float ease_exponential_in(float t) { return t == 0.0 ? t : pow(2.0, 10.0 * (t - 1.0)); }

float ease_exponential_out(float t) { return t == 1.0 ? t : 1.0 - pow(2.0, -10.0 * t); }

float ease_linear(float t) { return t; }

float ease_quadratic_in_out(float t)
{
	float p = 2.0 * t * t;
	return t < 0.5 ? p : -p + (4.0 * t) - 1.0;
}

float ease_quadratic_in(float t) { return t * t; }

float ease_quadratic_out(float t) { return -t * (t - 2.0); }

float ease_quartic_in_out(float t) { return t < 0.5 ? +8.0 * pow(t, 4.0) : -8.0 * pow(t - 1.0, 4.0) + 1.0; }

float ease_quartic_in(float t) { return pow(t, 4.0); }

float ease_quartic_out(float t) { return pow(t - 1.0, 3.0) * (1.0 - t) + 1.0; }

float ease_qintic_in_out(float t) { return t < 0.5 ? +16.0 * pow(t, 5.0) : -0.5 * pow(2.0 * t - 2.0, 5.0) + 1.0; }

float ease_qintic_in(float t) { return pow(t, 5.0); }

float ease_qintic_out(float t) { return 1.0 - (pow(t - 1.0, 5.0)); }

float ease_sine_in_out(float t) { return -0.5 * (cos(M_PI * t) - 1.0); }

float ease_sine_in(float t) { return sin((t - 1.0) * M_PI_2) + 1.0; }

float ease_sine_out(float t) { return sin(t * M_PI_2); }
