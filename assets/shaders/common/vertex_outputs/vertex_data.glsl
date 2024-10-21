#ifndef F_SH_VERTEX_DATA_GLS
#define F_SH_VERTEX_DATA_GLS

#include "/common/export.glsl"
#include "/lighting/lighting.glsl"
#include "/common/vertex_data_locations.glsl"

// This would be cleaner as a struct, but that has casued issues with OpenGL in the past.
layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS mat4 CONCAT(EXPORT_VS_PREFIX, M);
layout(location = SHADER_VERTEX_DATA_LOCATION +4) EXPORT_VS mat3 CONCAT(EXPORT_VS_PREFIX, TBN);

layout(location = SHADER_VERTEX_DATA_LOCATION +7) EXPORT_VS vec4 CONCAT(EXPORT_VS_PREFIX, color);
layout(location = SHADER_VERTEX_DATA_LOCATION +8) EXPORT_VS vec3 CONCAT(EXPORT_VS_PREFIX, vert_pos_ws);

layout(location = SHADER_VERTEX_DATA_LOCATION +9) EXPORT_VS vec3 CONCAT(EXPORT_VS_PREFIX, vert_normal);
layout(location = SHADER_VERTEX_DATA_LOCATION +10) EXPORT_VS vec3 CONCAT(EXPORT_VS_PREFIX, vert_normal_cs);

layout(location = SHADER_VERTEX_DATA_LOCATION +11) EXPORT_VS vec2 CONCAT(EXPORT_VS_PREFIX, vert_uv);
layout(location = SHADER_VERTEX_DATA_LOCATION +12) EXPORT_VS vec2 CONCAT(EXPORT_VS_PREFIX, vert_uv_lightmap);

layout(location = SHADER_VERTEX_DATA_LOCATION +13) EXPORT_VS float CONCAT(EXPORT_VS_PREFIX, wrinkleDelta);

// Note: If new data is added, make sure to update SHADER_USER1_LOCATION accordingly!

#ifdef GLS_FRAGMENT_SHADER

mat4 get_model_matrix() { return CONCAT(EXPORT_VS_PREFIX, M); }
mat3 get_tbn_matrix() { return CONCAT(EXPORT_VS_PREFIX, TBN); }

vec4 get_instance_color() { return CONCAT(EXPORT_VS_PREFIX, color); }
vec3 get_vertex_position_ws() { return CONCAT(EXPORT_VS_PREFIX, vert_pos_ws); }

vec3 get_vertex_normal() { return CONCAT(EXPORT_VS_PREFIX, vert_normal); }
vec3 get_vertex_normal_cs() { return CONCAT(EXPORT_VS_PREFIX, vert_normal_cs); }

vec2 get_vertex_uv() { return CONCAT(EXPORT_VS_PREFIX, vert_uv); }
vec2 get_vertex_uv_lightmap() { return CONCAT(EXPORT_VS_PREFIX, vert_uv_lightmap); }

float get_wrinkle_delta() { return CONCAT(EXPORT_VS_PREFIX, wrinkleDelta); }

#else

void set_model_matrix(mat4 value) { CONCAT(EXPORT_VS_PREFIX, M) = value; }
void set_tbn_matrix(mat3 value) { CONCAT(EXPORT_VS_PREFIX, TBN) = value; }

void set_instance_color(vec4 value) { CONCAT(EXPORT_VS_PREFIX, color) = value; }
void set_vertex_position_ws(vec3 value) { CONCAT(EXPORT_VS_PREFIX, vert_pos_ws) = value; }

void set_vertex_normal(vec3 value) { CONCAT(EXPORT_VS_PREFIX, vert_normal) = value; }
void set_vertex_normal_cs(vec3 value) { CONCAT(EXPORT_VS_PREFIX, vert_normal_cs) = value; }

void set_vertex_uv(vec2 value) { CONCAT(EXPORT_VS_PREFIX, vert_uv) = value; }
void set_vertex_uv_lightmap(vec2 value) { CONCAT(EXPORT_VS_PREFIX, vert_uv_lightmap) = value; }

void set_wrinkle_delta(float value) { CONCAT(EXPORT_VS_PREFIX, wrinkleDelta) = value; }

#endif

#endif
