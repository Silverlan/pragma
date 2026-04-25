#ifndef F_SH_TANGENTSPACE_GLS
#define F_SH_TANGENTSPACE_GLS

#include "/common/export.glsl"
#include "/common/vertex_data_locations.glsl"

#ifdef GLS_VERTEX_SHADER
layout(location = SHADER_NORMAL_TANGENT_BUFFER_LOCATION) in vec4 in_vert_tangent;
// layout(location = SHADER_NORMAL_BITANGENT_BUFFER_LOCATION) in vec3 in_vert_bitangent;
vec3 get_vertex_bitangent(vec3 normal)
{
	return cross(normal, vec3(in_vert_tangent)) *in_vert_tangent.w;
}
#endif

#endif
