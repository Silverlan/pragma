#ifndef F_VS_LIGHTMAP_GLS
#define F_VS_LIGHTMAP_GLS

#include "/common/inputs/lightmap_config.glsl"

#if ENABLE_LIGHTMAP == 1
#include "/common/vertex_outputs/vs_vertex_data_locations.glsl"
layout(location = SHADER_LIGHT_MAP_UV_BUFFER_LOCATION) in vec2 in_uv_lightmap;
#endif

#endif
