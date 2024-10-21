#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/alpha_mode.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	GUI_BASE_PUSH_CONSTANTS
	int alphaOnly;
	float lod;
	uint channels;
	uint alphaMode;
	float alphaCutoff;
}
u_pushConstants;

layout(location = 0) out vec4 fs_color;

vec4 get_texture_color(vec2 uv)
{
	if(u_pushConstants.lod < 0.0)
		return texture(u_texture, uv);
	return textureLod(u_texture, uv, u_pushConstants.lod);
}

void main()
{
	vec4 color;
	if(u_pushConstants.alphaOnly == 1) {
		color = u_pushConstants.color;
		color.a *= get_texture_color(vs_vert_uv).r;
	}
	else
		color = get_texture_color(vs_vert_uv).rgba * u_pushConstants.color;
	fs_color = vec4(color[(u_pushConstants.channels << 24) >> 24], color[(u_pushConstants.channels << 16) >> 24], color[(u_pushConstants.channels << 8) >> 24], color[u_pushConstants.channels >> 24]);
	fs_color.a = apply_alpha_mode(fs_color.a, u_pushConstants.alphaMode, u_pushConstants.alphaCutoff);
}
