#ifndef F_SH_COLOR_GLS
#define F_SH_COLOR_GLS

void rgba8_to_rgba16(inout vec4 col)
{
	const float sc = 255.0 / 65535.0;
	for(int i = 0; i < 4; ++i)
		col[i] /= sc;
}

void r8_to_r16(inout float v)
{
	const float sc = 255.0 / 65535.0;
	v /= sc;
}

/* RGB - HSV conversion */
// Source: http://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
vec3 rgb_to_hsv(vec3 c)
{
	vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv_to_rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
/* */

#endif
