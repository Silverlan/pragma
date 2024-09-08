#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 fs_color;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants { vec4 clearColor; }
u_pushConstants;

void main() { fs_color = u_pushConstants.clearColor; }
