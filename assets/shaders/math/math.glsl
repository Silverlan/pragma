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

#define FLT_EPSILON 0.00001

float length_sqr(vec3 v) { return pow2(v.x) + pow2(v.y) + pow2(v.z); }

float units_to_meters(float units) { return units * 0.025; }
float meters_to_units(float meters) { return meters / 0.025; }

bool compare(float a, float b) { return abs(a - b) < FLT_EPSILON; }
bool compare(vec3 a, vec3 b) { return compare(a.x, b.x) && compare(a.y, b.y) && compare(a.z, b.z); }

float atan2(in float y, in float x)
{
	// Source: https://stackoverflow.com/a/27228836
	return x == 0.0 ? sign(y) * M_PI / 2 : atan(y, x);
}

float calc_luminance(vec3 color) { return 0.212671 * color.r + 0.71516 * color.g + 0.072169 * color.b; }

float wrap(float value, float max, float min)
{
	float range = max -min;
	return (range != 0.0) ? value -(range *floor((value -min) /range)) : min;
}

vec3 wrap(vec3 value, vec3 max, vec3 min)
{
	return vec3(
		wrap(value.x, max.x, min.x),
		wrap(value.y, max.y, min.y),
		wrap(value.z, max.z, min.z)
	);
}

float pingpong(float a, float b)
{
	return (b != 0.0) ? abs(fract((a -b) / (b *2.0)) *b *2.0 -b) : 0.0;
}

float smoothmin(float a, float b, float k)
{
	if (k != 0.0) {
		float h = max(k -abs(a -b), 0.0) /k;
		return min(a, b) - h *h *h *k *(1.0 / 6.0);
	} else
		return min(a, b);
}

float floored_modulo(float a, float b)
{
	return (b != 0.0) ? a -floor(a / b) *b : 0.0;
}

vec3 project(vec3 v, vec3 v_proj)
{
	float len_squared = dot(v_proj, v_proj);
	return (len_squared != 0.0) ? (dot(v, v_proj) /len_squared) *v_proj : vec3(0.0);
}

#endif
