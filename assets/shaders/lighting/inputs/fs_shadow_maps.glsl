#ifndef F_FS_SHADOW_GLS
#define F_FS_SHADOW_GLS

#define SOFT_SHADOW_TYPE SOFT_SHADOW_PCF
#define SOFT_SHADOW_PCF_KERNEL 64

// PCF
#ifndef SOFT_SHADOW_PCF_KERNEL
#define SOFT_SHADOW_PCF_KERNEL 64 // 16, 32, 64 or 128
#endif
#if SOFT_SHADOW_PCF_KERNEL == 128
#define SOFT_SHADOW_PCF_KERNEL_ITER 4.5
#elif SOFT_SHADOW_PCF_KERNEL == 64
#define SOFT_SHADOW_PCF_KERNEL_ITER 3.5
#elif SOFT_SHADOW_PCF_KERNEL == 32
#define SOFT_SHADOW_PCF_KERNEL_ITER 2.5
#else
#define SOFT_SHADOW_PCF_KERNEL_ITER 1.5
#endif

#include "../shadow/fs_shadow_spot.glsl"
#include "../shadow/fs_shadow_point.glsl"
#include "../shadow/fs_shadow_csm.glsl"

layout(LAYOUT_ID(SHADOWS, MAPS)) uniform sampler2D shadowMaps[MAX_SHADOW_MAPS];
layout(LAYOUT_ID(SHADOWS, CUBEMAPS)) uniform samplerCube shadowCubemaps[MAX_SHADOW_CUBE_MAPS];

float calculate_shadow(shadowMapSamplerType shadowMap, vec4 shadowCoord, float bias) { return calculate_spot_shadow(shadowMap, shadowCoord, bias); }

float calculate_shadow(vec3 fragPosWs, shadowCubeMapSamplerType shadowMap, float fragDepth, vec3 lightPosition) { return calculate_point_shadow(fragPosWs, shadowMap, fragDepth, lightPosition); }

float calculate_shadow(shadowMapSamplerType shadowMap, mat4 vp, vec4 worldCoord, float bias) { return calculate_csm_shadow(shadowMap, vp, worldCoord, bias); }

float calculate_shadow(int index, vec4 worldCoord, float bias) { return calculate_csm_shadow(index, worldCoord, bias); }

#endif
