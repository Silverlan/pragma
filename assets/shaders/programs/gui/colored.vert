#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 in_vert_pos;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants {GUI_BASE_PUSH_CONSTANTS} u_pushConstants;

#include "vs_shared.glsl"

void main() { gl_Position = get_vertex_position(); }
