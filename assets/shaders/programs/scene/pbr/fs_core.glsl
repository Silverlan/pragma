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
		parallaxInfo.heightScale = get_parallax_height_scale();
		parallaxInfo.steps = get_parallax_steps();
		texCoords = apply_parallax(true, texCoords, parallaxInfo);
	}
	return texCoords;
}

vec4 calc_pbr(vec4 albedoColor, vec2 texCoords, uint debugMode)
{
	PbrMaterial pbrMat;
	pbrMat.color = u_material.material.color;
	pbrMat.roughnessFactor = u_material.material.roughnessFactor;
	pbrMat.metalnessFactor = u_material.material.metalnessFactor;
	pbrMat.aoFactor = u_material.material.aoFactor;
	pbrMat.alphaMode = u_material.material.alphaMode;
	pbrMat.alphaCutoff = u_material.material.alphaCutoff;
	return calc_pbr(albedoColor, texCoords, u_pushConstants.debugMode, pbrMat, u_material.material.emissionFactor.rgb, u_material.material.flags);
}

#endif
