#ifndef F_FS_PBR_TEXTURES_IBL_GLS
#define F_FS_PBR_TEXTURES_IBL_GLS

layout(LAYOUT_ID(PBR, IRRADIANCE_MAP)) uniform samplerCube u_irradianceMap;
layout(LAYOUT_ID(PBR, PREFILTER_MAP)) uniform samplerCube u_prefilterMap;
layout(LAYOUT_ID(PBR, BRDF_MAP)) uniform sampler2D u_brdfLUT;

#endif
