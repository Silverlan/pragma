#ifndef F_SH_VERTEX_DATA_GLS
#define F_SH_VERTEX_DATA_GLS

#include "/common/export.glsl"
#include "/lighting/lighting.glsl"
#include "/common/vertex_data_locations.glsl"

struct VertexData {
	mat4 M;
	vec4 color;
	mat3 TBN;
	vec3 vert_pos_ws; // Vertex Position in world space

	vec3 vert_normal;    // Vertex Normal
	vec3 vert_normal_cs; // Vertex Normal in camera space

	vec2 vert_uv;          // Texture coordinates
	vec2 vert_uv_lightmap; // Texture coordinates for light map atlas

	float wrinkleDelta;
};
layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VertexData
#ifdef GLS_FRAGMENT_SHADER
  fs_in
#else
  vs_out
#endif
  ;

#ifdef GLS_FRAGMENT_SHADER
#define VERTEX_DATA fs_in
#else
#define VERTEX_DATA vs_out
#endif

vec3 get_vertex_position_ws() { return VERTEX_DATA.vert_pos_ws; }
//vec3 get_vertex_position_cs() {return VERTEX_DATA.vert_pos_cs;}
vec3 get_vertex_normal() { return VERTEX_DATA.vert_normal; }
vec3 get_vertex_normal_cs() { return VERTEX_DATA.vert_normal_cs; }

#ifdef GLS_FRAGMENT_SHADER
mat4 get_model_matrix() { return fs_in.M; }
vec4 get_instance_color() { return fs_in.color; }
#endif

#endif
