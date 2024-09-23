#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 in_vert_pos;
layout(location = 1) in vec2 in_vert_uv;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	GUI_BASE_PUSH_CONSTANTS
	int alphaOnly;
	float lod;
	uint channels;
	uint alphaMode;
	float alphaCutoff;
}
u_pushConstants;

layout(location = 0) out vec2 vs_vert_uv;

#include "vs_shared.glsl"

void main()
{
	gl_Position = get_vertex_position();
	vs_vert_uv = in_vert_uv;
}
