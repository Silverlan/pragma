#ifndef F_FS_PBR_IBL_GLS
#define F_FS_PBR_IBL_GLS

#include "/common/pbr/material.glsl"
#include "/common/fs_tonemapping.glsl"
#include "/common/inputs/fs_ibl.glsl"

const float u_MipCount = 4.0; // TODO
#define USE_HDR
#define USE_TEX_LOD
vec3 get_ibl_contribution(MaterialInfo materialInfo, vec3 n, vec3 v, float reflectionProbeIntensity)
{
	float NdotV = clamp(dot(n, v), 0.0, 1.0);

	float lod = clamp(materialInfo.perceptualRoughness * float(u_MipCount), 0.0, float(u_MipCount));
	vec3 reflection = normalize(reflect(-v, n));

	vec2 brdfSamplePoint = clamp(vec2(NdotV, materialInfo.perceptualRoughness), vec2(0.0, 0.0), vec2(1.0, 1.0));
	// retrieve a scale and bias to F0. See [1], Figure 3
	vec2 brdf = texture(u_brdfLUT, brdfSamplePoint).rg;

	vec4 diffuseSample = texture(u_irradianceMap, n);

#ifdef USE_TEX_LOD
	// TODO: Use LOD!!
	vec4 specularSample = textureLod(u_prefilterMap, reflection, lod);
#else
	vec4 specularSample = texture(u_prefilterMap, reflection);
#endif
	specularSample.rgb *= reflectionProbeIntensity;

	vec3 diffuseLight = diffuseSample.rgb;
	vec3 specularLight = specularSample.rgb;

	vec3 diffuse = diffuseLight * materialInfo.diffuseColor;
	vec3 specular = specularLight * (materialInfo.specularColor * brdf.x + brdf.y);

	return diffuse + specular;
}

#endif
