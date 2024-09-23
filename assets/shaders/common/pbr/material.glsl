#ifndef F_FS_PBR_MATERIAL_GLS
#define F_FS_PBR_MATERIAL_GLS

#include "lighting/angular_info.glsl"
#include "/math/math.glsl"
#include "/common/rma.glsl"
#include "/common/fs_tonemapping.glsl"
#include "/common/alpha_mode.glsl"

#ifdef MATERIAL_WRINKLE_STRETCH_MAP_ENABLED
#include "/programs/scene/pbr/textures.glsl"
#endif

// Khronos
struct MaterialInfo {
	float perceptualRoughness; // roughness value, as authored by the model creator (input to shader)
	vec3 reflectance0;         // full reflectance color (normal incidence angle)

	float alphaRoughness; // roughness mapped to a more linear change in the roughness (proposed by [2])
	vec3 diffuseColor;    // color contribution from diffuse lighting

	vec3 reflectance90; // reflectance color at grazing angle
	vec3 specularColor; // color contribution from specular lighting

	float aoFactor;
};

// Lambert lighting
// see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
vec3 diffuse(MaterialInfo materialInfo) { return materialInfo.diffuseColor / M_PI; }

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specular_reflection(MaterialInfo materialInfo, AngularInfo angularInfo) { return materialInfo.reflectance0 + (materialInfo.reflectance90 - materialInfo.reflectance0) * pow(clamp(1.0 - angularInfo.VdotH, 0.0, 1.0), 5.0); }

// Smith Joint GGX
// Note: Vis = G / (4 * NdotL * NdotV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float visibility_occlusion(MaterialInfo materialInfo, AngularInfo angularInfo)
{
	float NdotL = angularInfo.NdotL;
	float NdotV = angularInfo.NdotV;
	float alphaRoughnessSq = materialInfo.alphaRoughness * materialInfo.alphaRoughness;

	float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
	float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

	float GGX = GGXV + GGXL;
	if(GGX > 0.0)
		return 0.5 / GGX;
	return 0.0;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacet_distribution(MaterialInfo materialInfo, AngularInfo angularInfo)
{
	float alphaRoughnessSq = materialInfo.alphaRoughness * materialInfo.alphaRoughness;
	float f = (angularInfo.NdotH * alphaRoughnessSq - angularInfo.NdotH) * angularInfo.NdotH + 1.0;
	return alphaRoughnessSq / (M_PI * f * f);
}

struct PbrMaterial {
	vec4 color;
	float roughnessFactor;
	float metalnessFactor;
	float aoFactor;
	uint alphaMode;
	float alphaCutoff;
};

MaterialInfo build_material_info(PbrMaterial materialData, uint materialFlags, vec4 albedoColor, vec4 instanceColor, vec4 rma, float wrinkleData, vec2 wrinkleTexCoords)
{
	// Metallic and Roughness material properties are packed together
	// In glTF, these factors can be specified by fixed scalar values
	// or from a metallic-roughness map
	float perceptualRoughness = 0.0;
	float metallic = 0.0;
	vec4 baseColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 diffuseColor = vec3(0.0);
	vec3 specularColor = vec3(0.0);
	vec3 f0 = vec3(0.04);

	// f0 = specular
	specularColor = f0;
	float oneMinusSpecularStrength = 1.0 - max(max(f0.r, f0.g), f0.b);
	diffuseColor = baseColor.rgb * oneMinusSpecularStrength;

	perceptualRoughness = rma[RMA_CHANNEL_ROUGHNESS] * 1.0;
	perceptualRoughness *= materialData.roughnessFactor;

	metallic = rma[RMA_CHANNEL_METALNESS] * materialData.metalnessFactor;

	vec4 colorMod = instanceColor;
	baseColor = albedoColor;
	if(is_material_translucent(materialFlags))
		baseColor.a = apply_alpha_mode(baseColor.a * colorMod.a * materialData.color.a, materialData.alphaMode, materialData.alphaCutoff);
	else
		baseColor.a = 1.0;
	baseColor.rgb *= colorMod.rgb * materialData.color.rgb;

#ifdef MATERIAL_WRINKLE_STRETCH_MAP_ENABLED
	if(use_wrinkle_maps(materialFlags)) {
		if(wrinkleData != 0.0) {
			float wrinkle = clamp(-wrinkleData, 0, 1);
			float stretch = clamp(wrinkleData, 0, 1);
			float baseColorFactor = 1.0 - wrinkle - stretch;

			vec4 wrinkleCol = texture(u_wrinkleCompressMap, wrinkleTexCoords);
			vec4 stretchCol = texture(u_wrinkleStretchMap, wrinkleTexCoords);

			baseColor.rgb = baseColorFactor * baseColor.rgb + wrinkle * wrinkleCol.rgb + stretch * stretchCol.rgb;
		}
	}
#endif

	//baseColor *= u_DiffuseFactor;
	diffuseColor = baseColor.rgb * (vec3(1.0) - f0) * (1.0 - metallic);
	specularColor = mix(f0, baseColor.rgb, metallic);

	perceptualRoughness = clamp(perceptualRoughness, 0.0, 1.0);
	metallic = clamp(metallic, 0.0, 1.0);

	// Roughness is authored as perceptual roughness; as is convention,
	// convert to material roughness by squaring the perceptual roughness [2].
	float alphaRoughness = perceptualRoughness * perceptualRoughness;

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	vec3 specularEnvironmentR0 = specularColor.rgb;
	// Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
	vec3 specularEnvironmentR90 = vec3(clamp(reflectance * 50.0, 0.0, 1.0));

	float aoFactor = materialData.aoFactor;
	return MaterialInfo(perceptualRoughness, specularEnvironmentR0, alphaRoughness, diffuseColor, specularEnvironmentR90, specularColor, aoFactor);
}

#endif
