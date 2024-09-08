#ifndef F_MATRIX_GLS
#define F_MATRIX_GLS

#include "math.glsl"

mat4 get_rotation_matrix(vec3 axis, float angle)
{
	// Source: http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0, oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0, oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s,
	  oc * axis.z * axis.z + c, 0.0, 0.0, 0.0, 0.0, 1.0);
}

mat3 get_rotation_matrix(vec4 q)
{
	return mat3(1.0 - 2.0 * pow2(q.y) - 2.0 * pow2(q.z), 2.0 * q.x * q.y + 2.0 * q.z * q.w, 2.0 * q.x * q.z - 2.0 * q.y * q.w, 2.0 * q.x * q.y - 2.0 * q.z * q.w, 1.0 - 2.0 * pow2(q.x) - 2.0 * pow2(q.z), 2.0 * q.y * q.z + 2.0 * q.x * q.w, 2.0 * q.x * q.z + 2.0 * q.y * q.w,
	  2.0 * q.y * q.z - 2.0 * q.x * q.w, 1.0 - 2.0 * pow2(q.x) - 2.0 * pow2(q.y));
}

mat4 scale_matrix(mat4 m, vec3 v)
{
	mat4 r = m;
	r[0] = m[0] * v[0];
	r[1] = m[1] * v[1];
	r[2] = m[2] * v[2];
	r[3] = m[3];
	return r;
}

#endif
