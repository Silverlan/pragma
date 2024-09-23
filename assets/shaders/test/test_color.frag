#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(LAYOUT_PUSH_CONSTANTS()) uniform RenderSettings { vec4 color; }
u_pushConstants;

layout(location = 0) out vec4 fs_color;

void main() { fs_color = u_pushConstants.color; }
