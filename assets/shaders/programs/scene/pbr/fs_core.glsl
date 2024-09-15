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

vec2 get_uv_coordinates()
{
	vec2 texCoords = fs_in.vert_uv;
	if(use_parallax_map(u_material.material.flags)) {
		ParallaxInfo parallaxInfo;
		parallaxInfo.heightScale = get_mat_parallax_height_scale();
		parallaxInfo.steps = get_mat_parallax_steps();
		texCoords = apply_parallax(true, texCoords, parallaxInfo);
	}
	return texCoords;
}

vec4 calc_pbr(vec4 albedoColor, vec2 texCoords, uint debugMode)
{
	PbrMaterial pbrMat;
	pbrMat.color = vec4(get_mat_color_factor(), get_mat_alpha_factor());
	pbrMat.roughnessFactor = get_mat_roughness_factor();
	pbrMat.metalnessFactor = get_mat_metalness_factor();
	pbrMat.aoFactor = get_mat_ao_factor();
	pbrMat.alphaMode = get_mat_alpha_mode();
	pbrMat.alphaCutoff = get_mat_alpha_cutoff();
	return calc_pbr(albedoColor, texCoords, u_pushConstants.debugMode, pbrMat, get_mat_emission_factor().rgb, get_mat_flags());
}

#endif
