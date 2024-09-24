#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "fs_core.glsl"

void main()
{
	vec2 uv = get_uv_coordinates();
	vec4 albedoColor = fetch_albedo_map(uv, get_instance_color());
	fs_color = calc_pbr(albedoColor, uv, u_pushConstants.debugMode);
	if(CSPEC_BLOOM_OUTPUT_ENABLED == 1)
		extract_bright_color(fs_color, u_renderer.bloomThreshold);
}
