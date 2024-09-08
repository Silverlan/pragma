#ifndef F_VS_PARTICLE_VERTEX_GLS
#define F_VS_PARTICLE_VERTEX_GLS

#include "particle_mode.glsl"
#include "/math/math.glsl"
#include "/math/half_float.glsl"
#include "/math/matrix.glsl"
#include "particle_instance.glsl"

layout(location = SHADER_POS_BUFFER_LOCATION) in vec3 in_pos;
layout(location = SHADER_RADIUS_BUFFER_LOCATION) in float in_radius;
layout(location = SHADER_PREVPOS_BUFFER_LOCATION) in vec3 in_prevPos;
layout(location = SHADER_AGE_BUFFER_LOCATION) in float in_age;
layout(location = SHADER_ANIMATION_FRAME_INDICES_LOCATION) in uint in_animFrameIndices;
layout(location = SHADER_ANIMATION_FRAME_INTERP_FACTOR_LOCATION) in float in_animInterpFactor;

#ifndef ENABLE_PARTICLE_ROTATION
#define ENABLE_PARTICLE_ROTATION 1
#endif

#ifndef ENABLE_PARTICLE_ORIENTATION
#define ENABLE_PARTICLE_ORIENTATION 1
#endif

#ifndef ENABLE_PARTICLE_LENGTH
#define ENABLE_PARTICLE_LENGTH 1
#endif

#if ENABLE_PARTICLE_ROTATION == 1
layout(location = SHADER_ROTATION_BUFFER_LOCATION) in float in_rotation;
float get_particle_rotation() { return in_rotation; }
#endif

#if ENABLE_PARTICLE_LENGTH == 1
layout(location = SHADER_LENGTH_YAW_BUFFER_LOCATION) in uint in_lengthSequence;
float get_particle_length() { return float16_to_float(int((in_lengthSequence << 16) >> 16)); }
float get_particle_rotation_yaw() { return float16_to_float(int(in_lengthSequence >> 16)); }
#endif

float get_particle_extent(float radius) { return sqrt(pow2(radius) * 2.0); } // Convert radius to box extent

vec3 get_particle_pos() { return in_pos; }
float get_particle_radius() { return in_radius; }
vec3 get_prev_particle_pos() { return in_prevPos.xyz; }
float get_particle_age() { return in_age; }

uint get_vertex_index()
{
	uint baseIndices[] = uint[](0, 1, 2, 3, 0, 2);
	return baseIndices[SH_VERTEX_INDEX];
}

vec2 get_vertex_quad_pos()
{
	vec2 squareVerts[] = vec2[](vec2(0.5, -0.5), vec2(-0.5, -0.5), vec2(-0.5, 0.5), vec2(0.5, 0.5));
	return squareVerts[get_vertex_index()];
}

vec3 get_corner_particle_vertex_position(vec3 origin
#if ENABLE_PARTICLE_ORIENTATION == 1
  ,
  int orientation
#endif
)
{
	vec3 particleCenterWs = get_particle_pos();
	vec2 vsize = vec2(get_particle_extent(get_particle_radius()), get_particle_extent(get_particle_length()));
	vec3 squareVert = vec3(get_vertex_quad_pos(), 0.0);

#if ENABLE_PARTICLE_ORIENTATION == 1
	vec3 right;
	vec3 up;
	switch(orientation) {
	case PARTICLE_ORIENTATION_TYPE_UPRIGHT:
		vec3 dir = u_instance.camUp_ws; // 'camUp_ws' is the particle world-rotation if this orientation type is selected
		right = cross(normalize(particleCenterWs - origin), dir);
		up = -dir;
		break;
	case PARTICLE_ORIENTATION_TYPE_STATIC:
		right = vec3(0, 1, 0);
		up = u_instance.camUp_ws;
		break;
	case PARTICLE_ORIENTATION_TYPE_WORLD:
		up = -normalize(u_instance.camUp_ws);
		right = -normalize(u_instance.camRight_ws);
		vsize = vec2(u_instance.nearZ, u_instance.farZ);
		break;
	default:
		right = u_instance.camRight_ws;
		up = u_instance.camUp_ws;
	}
#if ENABLE_PARTICLE_ROTATION == 1
	squareVert = (get_rotation_matrix(normalize(vec3(0, 0, 1)), radians(in_rotation)) * vec4(squareVert.xyz, 1)).xyz;
#endif
#else
	vec3 right = u_instance.camRight_ws;
	vec3 up = u_instance.camUp_ws;
#endif
	return right * squareVert.x * vsize.x + up * squareVert.y * vsize.y;
}

vec3 get_particle_vertex_position(vec3 origin
#if ENABLE_PARTICLE_ORIENTATION == 1
  ,
  int orientation
#endif
)
{
	return get_particle_pos() + get_corner_particle_vertex_position(origin, orientation);
}
vec3 get_prev_particle_vertex_position(vec3 origin
#if ENABLE_PARTICLE_ORIENTATION == 1
  ,
  int orientation
#endif
)
{
	return in_prevPos.xyz + get_corner_particle_vertex_position(origin, orientation);
}

#if ENABLE_PARTICLE_ORIENTATION == 1
vec3 get_corner_particle_vertex_position(vec3 origin) { return get_corner_particle_vertex_position(origin, u_instance.orientation); }
vec3 get_particle_vertex_position(vec3 origin) { return get_particle_vertex_position(origin, u_instance.orientation); }
vec3 get_prev_particle_vertex_position(vec3 origin) { return get_prev_particle_vertex_position(origin, u_instance.orientation); }
#endif

#endif
