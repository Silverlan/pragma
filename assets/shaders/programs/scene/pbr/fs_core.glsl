#ifndef F_FS_PBR_CORE_GLS
#define F_FS_PBR_CORE_GLS

#include "/common/vertex_outputs/vertex_data.glsl"
#include "fs_config.glsl"
#include "sh_pbr.glsl"
#include "textures.glsl"
#include "/common/pbr/material.glsl"
#include "/common/inputs/material.glsl"
#include "/common/inputs/textures/parallax_map.glsl"
#include "/common/pixel_outputs/fs_bloom_color.glsl"
#include "/common/material_flags.glsl"
#include "/common/pbr/fs_pbr.glsl"
#include "/programs/scene/scene_push_constants.glsl"

vec4 calc_pbr(vec4 albedoColor, vec2 texCoords, uint debugMode)
{
	PbrMaterial pbrMat;
	pbrMat.color = vec4(get_mat_color_factor(), get_mat_alpha_factor());

#ifdef MATERIAL_PROP_ROUGHNESS_FACTOR_ENABLED
	pbrMat.roughnessFactor = get_mat_roughness_factor();
#else
	pbrMat.roughnessFactor = 0.5;
#endif

#ifdef MATERIAL_PROP_METALNESS_FACTOR_ENABLED
	pbrMat.metalnessFactor = get_mat_metalness_factor();
#else
	pbrMat.metalnessFactor = 0.0;
#endif

#ifdef MATERIAL_PROP_AO_FACTOR_ENABLED
	pbrMat.aoFactor = get_mat_ao_factor();
#else
	pbrMat.aoFactor = 1.0;
#endif

#ifdef MATERIAL_PROP_EMISSION_FACTOR_ENABLED
	vec3 emissionFactor = get_mat_emission_factor().rgb;
#else
	vec3 emissionFactor = vec3(0.0, 0.0, 0.0);
#endif

	pbrMat.alphaMode = get_mat_alpha_mode();
	pbrMat.alphaCutoff = get_mat_alpha_cutoff();
	return calc_pbr(albedoColor, texCoords, u_pushConstants.debugMode, pbrMat, emissionFactor, get_mat_flags());
}

#endif
