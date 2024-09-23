#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;
layout(LAYOUT_ID(TEXTURE2, TEXTURE2)) uniform sampler2D u_texture2;

layout(location = 0) out vec4 fs_color;

void main()
{
	vec4 col0 = texture(u_texture, vs_vert_uv).rgba;
	vec4 col1 = texture(u_texture2, vs_vert_uv).rgba;

	float alpha = col1.a;
	fs_color = mix(col0, col1, alpha);
}
