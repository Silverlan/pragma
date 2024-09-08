#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define DEBUG_MODE DEBUG_MODE_NONE

#include "../sh_textured_blend.gls"
#include "pbr.gls"

#define DESCRIPTOR_SET_MATERIAL_BINDING_ALBEDO2_MAP (DESCRIPTOR_SET_MATERIAL_BINDING_LAST + 1)
#define DESCRIPTOR_SET_MATERIAL_BINDING_ALBEDO3_MAP (DESCRIPTOR_SET_MATERIAL_BINDING_ALBEDO2_MAP + 1)

layout(LAYOUT_ID(MATERIAL, ALBEDO_MAP2)) uniform sampler2D u_albedoMap2;
layout(LAYOUT_ID(MATERIAL, ALBEDO_MAP3)) uniform sampler2D u_albedoMap3;

void main()
{
	vec2 texCoords = apply_parallax(use_parallax_map(), fs_in.vert_uv);
	vec4 albedoColor = texture(u_albedoMap, texCoords);
	if(u_pushConstants.alphaCount > 0) {
		vec4 albedoColor2 = texture(u_albedoMap2, texCoords);
		if(u_pushConstants.alphaCount > 1) {
			vec4 albedoColor3 = texture(u_albedoMap3, texCoords);
			albedoColor = albedoColor * (1.0 - (fs_disp_in.alpha.x + fs_disp_in.alpha.y)) + albedoColor2 * fs_disp_in.alpha.x + albedoColor3 * fs_disp_in.alpha.y;
		}
		else
			albedoColor = albedoColor * (1.0 - fs_disp_in.alpha.x) + albedoColor2 * fs_disp_in.alpha.x;
	}

	fs_color = calc_pbr(albedoColor, texCoords, u_pushConstants.debugMode);
	extract_bright_color(fs_color);
}
