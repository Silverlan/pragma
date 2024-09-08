#ifndef F_FS_PBR_LIGHTING_LIGHT_SHARED_GLS
#define F_FS_PBR_LIGHTING_LIGHT_SHARED_GLS

#include "/common/pbr/lighting/angular_info.glsl"
#include "/common/pbr/material.glsl"

vec3 get_point_shade(vec3 pointToLight, MaterialInfo materialInfo, vec3 normal, vec3 view)
{
	AngularInfo angularInfo = get_angular_info(pointToLight, normal, view);
	angularInfo.NdotL = max(angularInfo.NdotL, 0.0);
	angularInfo.NdotV = max(angularInfo.NdotV, 0.0);

	// Calculate the shading terms for the microfacet specular shading model
	vec3 F = specular_reflection(materialInfo, angularInfo);
	float Vis = visibility_occlusion(materialInfo, angularInfo);
	float D = microfacet_distribution(materialInfo, angularInfo);

	// Calculation of analytical lighting contribution
	vec3 diffuseContrib = (1.0 - F) * diffuse(materialInfo);
	vec3 specContrib = F * Vis * D;

	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	return angularInfo.NdotL * (diffuseContrib + specContrib);
}

vec3 get_point_shade_lm(vec3 pointToLight, MaterialInfo materialInfo, vec3 normal, vec3 view, vec3 diffuse)
{
	AngularInfo angularInfo = get_angular_info(pointToLight, normal, view);
	angularInfo.NdotL = max(angularInfo.NdotL, 0.0);
	angularInfo.NdotV = max(angularInfo.NdotV, 0.0);

	// Calculate the shading terms for the microfacet specular shading model
	vec3 F = specular_reflection(materialInfo, angularInfo);
	float Vis = visibility_occlusion(materialInfo, angularInfo);
	float D = microfacet_distribution(materialInfo, angularInfo);

	// Calculation of analytical lighting contribution
	vec3 specContrib = F * Vis * D;

	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	return angularInfo.NdotL * (diffuse + specContrib);
}

#endif
