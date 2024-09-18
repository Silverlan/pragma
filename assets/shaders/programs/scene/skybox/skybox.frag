#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "skybox.glsl"
#include "/common/pixel_outputs/fs_bloom_color.glsl"

void main()
{
	fs_color = fetch_skybox(fs_in.cubemap_dir);
	fs_color *= fs_in.color;

	extract_bright_color(fs_color);
}