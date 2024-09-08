#ifndef F_SH_PARTICLE_BLOB_SHADOW_GLS
#define F_SH_PARTICLE_BLOB_SHADOW_GLS

#include "../../modules/sh_vertex_data_locations.gls"
#include "/common/export.glsl"
#include "../../modules/sh_light_source.gls"

#define SHADER_STORAGE_PARTICLE_SET 0
#define SHADER_STORAGE_PARTICLE_BINDING 0

#define SHADER_UNIFORM_LIGHT_DATA_SET (SHADER_STORAGE_PARTICLE_SET + 1)
#define SHADER_UNIFORM_LIGHT_DATA_BINDING 0

layout(push_constant) uniform Instance
{
	mat4 VP;
	vec3 camRight_ws;
	vec3 camUp_ws;
	vec4 lightPos;
}
u_instance;

#ifndef VERTEX_USER_DATA
#define VERTEX_USER_DATA
#endif

layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VS_OUT
{
	vec3 vert_pos_ws;  // Vertex Position in world space
	vec4 light_pos_ws; // Light Position in world space; 4th component stores the distance
	vec3 vert_pos_cs;
	flat uvec4 blobNeighbors;
}
#ifdef GLS_FRAGMENT_SHADER
fs_in
#else
vs_out
#endif
  ;

layout(std140, set = SHADER_UNIFORM_LIGHT_DATA_SET, binding = SHADER_UNIFORM_LIGHT_DATA_BINDING) uniform LightSource { LightSourceData data; }
u_lightSource;

#endif
