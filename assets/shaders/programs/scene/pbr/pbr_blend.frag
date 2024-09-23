#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define DEBUG_MODE DEBUG_MODE_NONE

#include "/programs/scene/textured_blend.glsl"
#include "fs_core.glsl"

void main()
{
	vec2 uv = get_uv_coordinates();
	vec4 instanceColor = get_instance_color();
	vec4 albedoColor = fetch_albedo_map(uv, instanceColor);
	if(u_pushConstants.alphaCount > 0) {
		vec4 albedoColor2 = fetch_albedo_map2(uv, instanceColor);
		if(u_pushConstants.alphaCount > 1) {
			vec4 albedoColor3 = fetch_albedo_map3(uv, instanceColor);
			albedoColor = albedoColor * (1.0 - (fs_disp_in.alpha.x + fs_disp_in.alpha.y)) + albedoColor2 * fs_disp_in.alpha.x + albedoColor3 * fs_disp_in.alpha.y;
		}
		else
			albedoColor = albedoColor * (1.0 - fs_disp_in.alpha.x) + albedoColor2 * fs_disp_in.alpha.x;
	}

	fs_color = calc_pbr(albedoColor, uv, u_pushConstants.debugMode);
	extract_bright_color(fs_color);
}
