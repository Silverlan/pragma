#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "textured_blend.glsl"
#include "vs_config.glsl"
#include "vs_world.glsl"
#include "/common/inputs/material.glsl"

layout(location = SHADER_ALPHA_BUFFER_LOCATION) in vec2 in_vertexAlpha;

void main() {
	export_world_fragment_data(use_normal_map(), use_parallax_map());
	vs_disp_out.alpha = in_vertexAlpha;
}
