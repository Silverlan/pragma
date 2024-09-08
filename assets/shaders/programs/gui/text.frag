#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, GLYPH_MAP)) uniform sampler2D u_texture;

layout(location = 0) out float fs_color;

void main()
{
	float r = texture(u_texture, vs_vert_uv).r;
	if(r == 0.0)
		discard;
	fs_color = r;
}
