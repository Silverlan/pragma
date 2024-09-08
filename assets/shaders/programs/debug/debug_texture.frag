#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "debug.glsl"

layout(location = 0) out vec4 fs_color;
layout(location = 1) out vec4 fs_brightColor;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;

void main()
{
	fs_color = texture(u_texture, fs_in.vert_uv);
	fs_color.a = clamp(max(max(fs_color.r, fs_color.g), fs_color.b), 0, 1); // Black to alpha
	fs_brightColor = fs_color;
}
