#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURES, SCENE)) uniform sampler2D u_texture;
layout(LAYOUT_ID(TEXTURES, BLOOM)) uniform sampler2D u_bloom;
layout(LAYOUT_ID(TEXTURES, GLOW)) uniform sampler2D u_glow;

layout(LAYOUT_PUSH_CONSTANTS()) uniform RenderSettings
{
	float exposure;
	float bloomScale;
	float glowScale; // Obsolete
	uint toneMapping;
	uint flipVertically;
}
u_renderSettings;

layout(location = 0) out vec4 fs_color;

#include "/common/fs_tonemapping.glsl"

layout(constant_id = 0) const uint CSPEC_BLOOM_ENABLED = 1;
layout(constant_id = 1) const uint CSPEC_FXAA_ENABLED = 1;
layout(constant_id = 2) const uint CSPEC_GLOW_ENABLED = 0;

void main()
{
	vec2 uv = vec2(vs_vert_uv.x, vs_vert_uv.y);
	if(u_renderSettings.flipVertically == 1)
		uv.y = 1.0 - uv.y;
	vec4 col = texture(u_texture, uv);
	if(CSPEC_BLOOM_ENABLED == 1) {
		vec3 colBloom = texture(u_bloom, uv).rgb;
		col.rgb += colBloom * u_renderSettings.bloomScale;
	}
	if(CSPEC_GLOW_ENABLED == 1) {
		vec3 colGlow = texture(u_glow, uv).rgb;
		col.rgb += colGlow;
	}

	fs_color = vec4(apply_tone_mapping(col.rgb, u_renderSettings.toneMapping, u_renderSettings.exposure), col.a);
	if(CSPEC_FXAA_ENABLED == 1)
		fs_color.a = dot(fs_color.rgb, vec3(0.299, 0.587, 0.114)); // Compute luma (required for fxaa)
	else
		fs_color.a = 1.0;
}
