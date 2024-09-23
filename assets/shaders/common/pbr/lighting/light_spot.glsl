#ifndef F_FS_PBR_LIGHTING_LIGHT_SPOT_GLS
#define F_FS_PBR_LIGHTING_LIGHT_SPOT_GLS

#include "light_shared.glsl"

// https://github.com/KhronosGroup/glTF/blob/master/extensions/2.0/Khronos/KHR_lights_punctual/README.md#inner-and-outer-cone-angles
float get_spot_attenuation(vec3 pointToLight, vec3 spotDirection, float outerConeCos, float innerConeCos)
{
	float actualCos = clamp(dot(spotDirection, -pointToLight), outerConeCos, innerConeCos);
	return smoothstep(outerConeCos, innerConeCos, actualCos);
}

float calc_spot_light_attenuation(vec3 pointToLight, LightSourceData light)
{
	float distance = length(pointToLight);
	pointToLight /= distance;
	float rangeAttenuation = calc_light_falloff(distance, light.position.w);
	rangeAttenuation = pow(rangeAttenuation, light.falloffExponent);
	return get_spot_attenuation(pointToLight, light.direction.xyz, cos(light.cutoffOuter), cos(light.cutoffInner)) * rangeAttenuation;
}

float calc_spot_light_attenuation(LightSourceData light, vec3 vertPos)
{
	vec3 pointToLight = light.position.xyz - vertPos;
	return calc_spot_light_attenuation(pointToLight, light);
}

vec3 apply_spot_light(LightSourceData light, MaterialInfo materialInfo, vec3 normal, vec3 view, vec3 vertPos)
{
	vec3 pointToLight = light.position.xyz - vertPos;
	float spotAttenuation = calc_spot_light_attenuation(pointToLight, light);
	vec3 shade = get_point_shade(normalize(pointToLight), materialInfo, normal, view);
	return spotAttenuation * light.color.a * light.color.rgb * shade;
}

#endif
