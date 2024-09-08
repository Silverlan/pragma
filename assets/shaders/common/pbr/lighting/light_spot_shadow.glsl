#ifndef F_FS_PBR_LIGHTING_LIGHT_SPOT_SHADOW_GLS
#define F_FS_PBR_LIGHTING_LIGHT_SPOT_SHADOW_GLS

#include "light_spot.glsl"
#include "/lighting/fs_lighting_spot.glsl"

vec3 apply_spot_light(LightSourceData light, uint lightIndex, MaterialInfo materialInfo, vec3 normal, vec3 view, vec3 vertPos, bool enableShadows)
{
	vec3 col = apply_spot_light(light, materialInfo, normal, view, vertPos);
	float shadowFactor = 1.0;
	if(CSPEC_ENABLE_DYNAMIC_SHADOWS == 1)
		shadowFactor = enableShadows ? get_spot_light_shadow_factor(lightIndex, true) : 1.0;
	return col * shadowFactor;
}

#endif
