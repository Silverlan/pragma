#ifndef F_SH_MATH_GLS
#define F_SH_MATH_GLS

#define M_PI 3.1415926535897932384626433832795
#define M_PI_2 (M_PI / 2.0)
#define M_PI_4 (M_PI / 4.0)

#define pow2(x) ((x) * (x))
#define pow3(x) ((x) * (x) * (x))
#define pow4(x) ((x) * (x) * (x) * (x))
#define pow5(x) ((x) * (x) * (x) * (x) * (x))
#define pow6(x) ((x) * (x) * (x) * (x) * (x) * (x))

#define cbrt(x) pow(x, 1.0 / 3.0)

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308

#define MIN_INT -2147483648
#define MAX_INT 2147483647

#define MIN_UINT 0
#define MAX_UINT 4294967295

float length_sqr(vec3 v) { return pow2(v.x) + pow2(v.y) + pow2(v.z); }

float units_to_meters(float units) { return units * 0.025; }
float meters_to_units(float meters) { return meters / 0.025; }

float atan2(in float y, in float x)
{
	// Source: https://stackoverflow.com/a/27228836
	return x == 0.0 ? sign(y) * M_PI / 2 : atan(y, x);
}

float calc_luminance(vec3 color) { return 0.212671 * color.r + 0.71516 * color.g + 0.072169 * color.b; }

#endif
