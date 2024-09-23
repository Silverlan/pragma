#ifndef F_SH_PREPASS_VERTEX_DATA_GLS
#define F_SH_PREPASS_VERTEX_DATA_GLS

#include "/common/vertex_data_locations.glsl"

struct VertexData {
	vec3 vert_normal_cs; // Vertex Normal in camera space
	vec2 vert_uv;
};
layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VertexData
#ifdef GLS_FRAGMENT_SHADER
  fs_in
#else
  vs_out
#endif
  ;

#endif
