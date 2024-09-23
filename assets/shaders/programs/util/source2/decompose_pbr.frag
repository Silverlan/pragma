#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/rma.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, ALBEDO)) uniform sampler2D u_albedoMap;
layout(LAYOUT_ID(TEXTURE, NORMAL)) uniform sampler2D u_normalMap;
layout(LAYOUT_ID(TEXTURE, ANISOTROPIC_GLOSSINESS)) uniform sampler2D u_aniGlossMap;
layout(LAYOUT_ID(TEXTURE, AMBIENT_OCCLUSION)) uniform sampler2D u_aoMap;

layout(location = 0) out vec4 fs_metallicRoughness;
layout(location = 1) out vec4 fs_albedo;

#define FSETTING_TREAT_ALPHA_AS_TRANSPARENCY 1
#define FSETTING_SPECULAR_WORKFLOW (FSETTING_TREAT_ALPHA_AS_TRANSPARENCY << 1)
#define FSETTING_TREAT_ALPHA_AS_SSS (FSETTING_SPECULAR_WORKFLOW << 1)
layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants { uint flags; }
u_pushConstants;

void main()
{
	vec4 albedoColor = texture(u_albedoMap, vs_vert_uv);
	vec4 normal = textureLod(u_normalMap, vs_vert_uv, 0); // We have to grab the normal values from lod 0, unsure why

	float roughness = normal.b;
	float metalness = albedoColor.a;
	float transparency = 1.0;
	float subsurfaceScattering = 1.0;
	if((u_pushConstants.flags & FSETTING_TREAT_ALPHA_AS_TRANSPARENCY) != 0) {
		transparency = metalness;
		metalness = 0.0;
	}
	else if((u_pushConstants.flags & FSETTING_TREAT_ALPHA_AS_SSS) != 0) {
		subsurfaceScattering = metalness;
		metalness = 0.0;
	}
	if((u_pushConstants.flags & FSETTING_SPECULAR_WORKFLOW) != 0) {
		vec4 gloss = textureLod(u_aniGlossMap, vs_vert_uv, 0);
		roughness = 1.0 - gloss.b;
	}
	float ao = texture(u_aoMap, vs_vert_uv).r;

	fs_metallicRoughness = vec4(0, 0, 0, 1);
	fs_metallicRoughness[RMA_CHANNEL_AO] = ao.r;
	fs_metallicRoughness[RMA_CHANNEL_ROUGHNESS] = roughness;
	fs_metallicRoughness[RMA_CHANNEL_METALNESS] = metalness;
	fs_metallicRoughness[3] = subsurfaceScattering; // TODO: This should be somewhere else!

	fs_albedo = vec4(albedoColor.rgb, transparency);
}
