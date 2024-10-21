#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;

layout(location = 0) out vec4 fs_color;

void main() { fs_color = texture(u_texture, vs_vert_uv).rgba; }
