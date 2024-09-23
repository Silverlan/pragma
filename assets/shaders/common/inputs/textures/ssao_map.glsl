#ifndef F_FS_SSAO_MAP_GLS
#define F_FS_SSAO_MAP_GLS

#ifndef ENABLE_SSAO
#define ENABLE_SSAO 1
#endif
#if ENABLE_SSAO == 1
layout(LAYOUT_ID(RENDERER, SSAO_MAP)) uniform sampler2D u_ssao;
#endif

#endif
