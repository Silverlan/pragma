#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/vertex_outputs/vertex_data.glsl"
#include "/common/inputs/fs_renderer.glsl"
#include "/common/pixel_outputs/fs_bloom_color.glsl"
#include "/programs/scene/pbr/fs_config.glsl"

void main()
{
	fs_color = fetch_albedo_map(get_vertex_uv(), get_instance_color());
	if(CSPEC_BLOOM_OUTPUT_ENABLED == 1)
		extract_bright_color(fs_color, u_renderer.bloomThreshold);
}
