#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define DEBUG_MODE DEBUG_MODE_NONE

#include "/common/vertex_outputs/vertex_data.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "/common/inputs/textures/albedo_map.glsl"
#include "/common/pbr/fs_util.glsl"
#include "/common/pixel_outputs/fs_bloom_color.glsl"
#include "/programs/scene/pbr/fs_config.glsl"

void main()
{
	vec4 albedoColor = texture(u_albedoMap, fs_in.vert_uv);
	vec4 baseColor = get_base_color(albedoColor);
	fs_color = baseColor;
	if(CSPEC_BLOOM_OUTPUT_ENABLED == 1) {
		fs_color = get_emission_color(fs_color, baseColor, fs_in.vert_uv);
		extract_bright_color(fs_color, u_renderer.bloomThreshold);
	}
}
