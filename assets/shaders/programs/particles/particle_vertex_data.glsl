#ifndef F_SH_PARTICLE_VERTEX_DATA_GLS
#define F_SH_PARTICLE_VERTEX_DATA_GLS

#include "/common/export.glsl"
#include "/common/vertex_data_locations.glsl"

#ifndef VERTEX_USER_DATA
#define VERTEX_USER_DATA
#endif

layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VS_OUT
{
	vec2 vert_uv;
	vec4 particle_color;
#if LIGHTING_ENABLED == 1
	vec3 vert_pos_ws;
	flat uint animationFrameIndices;
	vec3 vert_pos_cs;
	flat float animationFrameInterpFactor;
	vec3 vert_normal;
	vec3 vert_normal_cs;
#endif
	VERTEX_USER_DATA
}
#ifdef GLS_FRAGMENT_SHADER
fs_in
#else
vs_out
#endif
  ;

mat4 get_model_matrix() { return mat4(1.0); }

#ifdef GLS_FRAGMENT_SHADER
vec3 get_vertex_position_ws() { return fs_in.vert_pos_ws; }

vec3 get_vertex_normal() { return fs_in.vert_normal; }
vec3 get_vertex_normal_cs() { return fs_in.vert_normal_cs; }

vec2 get_vertex_uv() { return fs_in.vert_uv; }
#endif

#endif
