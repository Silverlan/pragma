#ifndef F_GS_PARTICLE_POLYBOARD_SHARED_GLS
#define F_GS_PARTICLE_POLYBOARD_SHARED_GLS

#include "/common/export.glsl"
#include "/math/math.glsl"
#include "../../modules/sh_camera_info.gls"

#define PARTICLE_MAX_CURVE_VERTICES 38

void emit_segment(vec3 r10, vec3 r11, vec3 r20, vec3 r21)
{
	emit_vertex(r10, vec2(0.0, 0.0));
	emit_vertex(r11, vec2(1.0, 0.0));
	emit_vertex(r21, vec2(1.0, 1.0));
	EndPrimitive();

	emit_vertex(r21, vec2(1.0, 1.0));
	emit_vertex(r20, vec2(0.0, 1.0));
	emit_vertex(r10, vec2(0.0, 0.0));
	EndPrimitive();
}

vec4 get_hermite_transform(float s) { return vec4(2.0 * pow3(s) - 3.0 * pow2(s) + 1.0, -2.0 * pow3(s) + 3.0 * pow2(s), pow3(s) - 2.0 * pow2(s) + s, pow3(s) - pow2(s)); }

void emit_curve_segment(float s, float sNext, vec3 r10, vec3 r11, vec3 r20, vec3 r21, vec3 t1, vec3 t2)
{
	vec4 h1 = get_hermite_transform(s);
	vec4 h2 = get_hermite_transform(sNext);

	vec3 p10 = h1[0] * r10 + h1[1] * r20 + h1[2] * t1 + h1[3] * t2;
	vec3 p11 = h1[0] * r11 + h1[1] * r21 + h1[2] * t1 + h1[3] * t2;

	vec3 p20 = h2[0] * r10 + h2[1] * r20 + h2[2] * t1 + h2[3] * t2;
	vec3 p21 = h2[0] * r11 + h2[1] * r21 + h2[2] * t1 + h2[3] * t2;
	emit_segment(p10, p11, p20, p21);
}

void process_curve_segment(vec3 origin, float sc)
{
	vec4 p0 = gl_in[0].gl_Position; // Previous Point
	vec4 p1 = gl_in[1].gl_Position; // Current Point
	vec4 p2 = gl_in[2].gl_Position; // Next Point
	vec4 p3 = gl_in[3].gl_Position; // Second Next Point

	float radius = u_instance.radius;

	// Current segment
	vec3 dirToCam = normalize(p1.xyz - origin);
	vec3 t = normalize(p2.xyz - p0.xyz);
	vec3 tdir = cross(t, dirToCam) * sc;
	vec3 r10 = p1.xyz + tdir * radius;
	vec3 r11 = p1.xyz - tdir * radius;

	// Next segment
	vec3 dirToCam2 = normalize(p2.xyz - origin);
	vec3 t2 = normalize(p3.xyz - p1.xyz);
	vec3 tdir2 = cross(t2, dirToCam2) * sc;
	vec3 r20 = p2.xyz + tdir2 * radius;
	vec3 r21 = p2.xyz - tdir2 * radius;

	// Generate curve segments
	vec3 ct1 = u_instance.curvature * (p2.xyz - p0.xyz); // TODO
	vec3 ct2 = u_instance.curvature * (p3.xyz - p1.xyz);

	float segmentCount = 6;
	float inc = 1.0 / float(segmentCount);
	for(float s = 0.0; s < 1.0; s += inc)
		emit_curve_segment(s, s + inc, r10, r11, r20, r21, ct1, ct2);
	//
}
void process_curve_segment(vec3 origin) { process_curve_segment(origin, 1.0); }

#endif
