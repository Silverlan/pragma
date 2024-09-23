#ifndef F_SH_QUATERNION_GLS
#define F_SH_QUATERNION_GLS

// Source: http://www.geeks3d.com/20141201/how-to-rotate-a-vertex-by-a-quaternion-in-glsl/
vec3 rotate_vector(vec3 v, vec4 r) { return v + 2.0 * cross(r.xyz, cross(r.xyz, v) + r.w * v); }

#endif
