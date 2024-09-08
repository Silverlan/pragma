#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "debug.glsl"

layout(location = 0) out vec4 fs_color;
layout(location = 1) out vec4 fs_brightColor;

void main()
{
	fs_color = fs_in.frag_col;
	fs_brightColor = fs_color;
}