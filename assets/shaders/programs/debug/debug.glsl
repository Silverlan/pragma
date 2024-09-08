#ifndef F_SH_DEBUG_GLS
#define F_SH_DEBUG_GLS

#define SHADER_VERTEX_DATA_LOCATION 0

#include "/common/export.glsl"

layout(location = SHADER_VERTEX_DATA_LOCATION) EXPORT_VS VS_OUT
{
	vec4 frag_col;
	vec2 vert_uv;
}
#ifdef GLS_FRAGMENT_SHADER
fs_in
#else
vs_out
#endif
  ;

#endif
