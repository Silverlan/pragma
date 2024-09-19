#ifndef F_SH_TEXTURED_BLEND_GLS
#define F_SH_TEXTURED_BLEND_GLS

#define PUSH_USER_CONSTANTS int alphaCount;

#include "/common/vertex_outputs/vs_vertex_data_locations.glsl"
#include "/common/vertex_data_locations.glsl"
#include "/common/export.glsl"

#define SHADER_ALPHA_BUFFER_LOCATION SHADER_USER_BUFFER1_LOCATION

#define SHADER_ALPHA1_LOCATION SHADER_USER1_LOCATION

layout(location = SHADER_ALPHA1_LOCATION) EXPORT_VS VS_DISP_OUT { vec2 alpha; }
#ifdef GLS_FRAGMENT_SHADER
fs_disp_in
#else
vs_disp_out
#endif
  ;

#endif
