#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 fs_color;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants {GUI_BASE_PUSH_CONSTANTS} u_pushConstants;

void main() { fs_color = u_pushConstants.color; }