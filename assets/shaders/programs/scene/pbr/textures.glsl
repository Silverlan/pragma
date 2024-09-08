#ifndef F_FS_PBR_TEXTURES_GLS
#define F_FS_PBR_TEXTURES_GLS

layout(LAYOUT_ID(MATERIAL, RMA_MAP)) uniform sampler2D u_rmaMap;
layout(LAYOUT_ID(MATERIAL, EXPONENT_MAP)) uniform sampler2D u_exponentMap;

#include "/common/inputs/textures/albedo_map.glsl"
#include "/common/inputs/textures/normal_map.glsl"

#endif
