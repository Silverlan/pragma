#ifndef F_SH_SHADOW_CSM_GLS
#define F_SH_SHADOW_CSM_GLS

#ifdef SHADOW_CSM_ENABLE_UV

#include "/common/export.glsl"
#include "../modules/sh_vertex_data_locations.gls"

struct VertexData {
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

#endif
