#ifndef F_FS_PBR_LIGHTING_LIGHT_DIRECTIONAL_GLS
#define F_FS_PBR_LIGHTING_LIGHT_DIRECTIONAL_GLS

#include "light_shared.glsl"
#include "/lighting/fs_lighting_directional.glsl"

vec3 apply_directional_light(LightSourceData light, uint lightIndex, MaterialInfo materialInfo, vec3 normal, vec3 view, bool enableShadows)
{
	vec3 pointToLight = -light.direction.xyz;
	vec3 shade = get_point_shade(pointToLight, materialInfo, normal, view);
	float shadowFactor = 1.0;
	if(CSPEC_ENABLE_DYNAMIC_SHADOWS == 1)
		shadowFactor = enableShadows ? get_directional_light_shadow_factor(lightIndex) : 1.0;
	return light.color.a * light.color.rgb * shade * shadowFactor;
}

#endif
