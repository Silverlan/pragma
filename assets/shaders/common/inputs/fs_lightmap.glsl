#ifndef F_FS_LIGHTMAP_GLS
#define F_FS_LIGHTMAP_GLS

#include "lightmap_config.glsl"

#if ENABLE_LIGHTMAP == 1

layout(LAYOUT_ID(RENDERER, LIGHTMAP)) uniform sampler2D u_lightMap;
layout(LAYOUT_ID(RENDERER, INDIRECT_LIGHTMAP)) uniform sampler2D u_lightMapIndirect;
layout(LAYOUT_ID(RENDERER, DIRECTIONAL_LIGHTMAP)) uniform sampler2D u_lightMapDominant;
#endif

#endif
