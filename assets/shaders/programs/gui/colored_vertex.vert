#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 in_vert_pos;
layout(location = 1) in vec4 in_vert_col;

layout(location = 0) out vec4 vs_vert_col;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants {GUI_BASE_PUSH_CONSTANTS} u_pushConstants;

#include "vs_shared.glsl"

vec4 get_line_vertex_position(vec2 vertPos) { return get_vertex_position(vertPos * 2.0 - vec2(1.0, 1.0)); }
void main()
{
	gl_Position = get_line_vertex_position(in_vert_pos);
	vs_vert_col = in_vert_col;
}
