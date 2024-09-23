#ifndef F_SH_GLOW_GLS
#define F_SH_GLOW_GLS

#define SHADER_VERTEX_DATA_LOCATION 0

#include "/common/export.glsl"

layout(location = 4) EXPORT_VS VS_OUT { vec2 vert_uv; }
#ifdef GLS_FRAGMENT_SHADER
fs_in
#else
vs_out
#endif
  ;

#endif
