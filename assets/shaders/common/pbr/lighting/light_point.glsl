#ifndef F_FS_PBR_LIGHTING_LIGHT_POINT_GLS
#define F_FS_PBR_LIGHTING_LIGHT_POINT_GLS

#include "light_shared.glsl"
#include "/lighting/fs_lighting_point.glsl"

float calc_point_light_attenuation(vec3 pointToLight, LightSourceData light)
{
	float distance = length(pointToLight);
	pointToLight /= distance;
	float rangeAttenuation = calc_light_falloff(distance, light.position.w);
	rangeAttenuation = pow(rangeAttenuation, light.falloffExponent);
	return rangeAttenuation;
}

float calc_point_light_attenuation(LightSourceData light, vec3 vertPos)
{
	vec3 pointToLight = light.position.xyz - vertPos;
	return calc_point_light_attenuation(pointToLight, light);
}

vec3 apply_point_light(LightSourceData light, uint lightIndex, MaterialInfo materialInfo, vec3 normal, vec3 view, vec3 vertPos, bool enableShadows)
{
	vec3 pointToLight = light.position.xyz - vertPos;
	float distance = length(pointToLight);
	pointToLight /= distance;
	float attenuation = calc_light_falloff(distance, light.position.w);
	attenuation = pow(attenuation, light.falloffExponent);
	vec3 shade = get_point_shade(pointToLight, materialInfo, normal, view);

	float shadowFactor = 1.0;
	if(CSPEC_ENABLE_DYNAMIC_SHADOWS == 1)
		shadowFactor = enableShadows ? get_point_light_shadow_factor(lightIndex, true, vertPos) : 1.0;
	return attenuation * light.color.a * light.color.rgb * shade * shadowFactor;
}

#endif
