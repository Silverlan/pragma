#ifndef F_SH_PARTICLE_POLYBOARD_GLS
#define F_SH_PARTICLE_POLYBOARD_GLS

#ifndef LIGHTING_ENABLED
#define LIGHTING_ENABLED 1
#define LIGHTING_USE_INCIDENCE_ANGLE 0
#endif
#define USE_LIGHT_DIR_TS 0

#define USE_NORMAL_MAP 0
#define USE_SPECULAR_MAP 0

#define SHADER_VERTEX_DATA_LOCATION 0

#define SHADER_UNIFORM_PARTICLE_MAP_SET 0
#define SHADER_UNIFORM_PARTICLE_MAP_BINDING 0

#define SHADER_UNIFORM_SCENE_DEPTH_MAP_SET (SHADER_UNIFORM_PARTICLE_MAP_SET + 1)
#define SHADER_UNIFORM_SCENE_DEPTH_MAP_BINDING 0

#define SHADER_UNIFORM_TEXTURE_ANIMATION_SET (SHADER_UNIFORM_SCENE_DEPTH_MAP_SET + 1)
#define SHADER_UNIFORM_TEXTURE_ANIMATION_BINDING 0

#define SHADER_UNIFORM_CAMERA_SET (SHADER_UNIFORM_TEXTURE_ANIMATION_SET + 1)
#define SHADER_UNIFORM_CAMERA_BINDING 0

#define SHADER_UNIFORM_RENDER_SETTINGS_SET SHADER_UNIFORM_CAMERA_SET
#define SHADER_UNIFORM_RENDER_SETTINGS_BINDING 1

#define SHADER_UNIFORM_TIME_SET (SHADER_UNIFORM_CAMERA_SET + 1)
#define SHADER_UNIFORM_TIME_BINDING 0

#define SHADER_UNIFORM_CSM_SET SHADER_UNIFORM_TIME_SET
#define SHADER_UNIFORM_CSM_BINDING 1

#include "/common/export.glsl"

#if LIGHTING_ENABLED == 1
mat4 get_model_matrix() { return mat4(1.0); } // Particle positions are already in world-space
#endif

#ifndef GLS_FRAGMENT_SHADER

layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VS_OUT
{
	vec4 particle_color;
	float particle_start;
#if LIGHTING_ENABLED == 1
	vec3 vert_pos_ws;
	vec3 vert_pos_cs;
	vec3 vert_normal;
	vec3 vert_normal_cs;
#endif
}
#ifdef GLS_GEOMETRY_SHADER
gs_in[]
#else
vs_out
#endif
  ;

#endif

#ifndef GLS_VERTEX_SHADER

layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_GS GS_OUT
{
	vec4 particle_color;
	float particle_start;
	vec2 vert_uv;
#if LIGHTING_ENABLED == 1
	vec3 vert_pos_ws;
	vec3 vert_pos_cs;
	vec3 vert_normal;
	vec3 vert_normal_cs;
#endif
}
#ifdef GLS_FRAGMENT_SHADER
fs_in
#else
gs_out[]
#endif
  ;

#endif

#endif
