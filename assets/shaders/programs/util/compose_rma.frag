#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/rma.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURES, ROUGHNESS)) uniform sampler2D u_roughnessMap;
layout(LAYOUT_ID(TEXTURES, METALNESS)) uniform sampler2D u_metalnessMap;
layout(LAYOUT_ID(TEXTURES, AMBIENT_OCCLUSION)) uniform sampler2D u_aoMap;

layout(location = 0) out vec4 fs_rma;

#define FSETTING_USE_SPECULAR_WORKFLOW 1
layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants { uint flags; }
u_pushConstants;

void main()
{
	float roughness = texture(u_roughnessMap, vs_vert_uv)[RMA_CHANNEL_ROUGHNESS];
	if((u_pushConstants.flags & FSETTING_USE_SPECULAR_WORKFLOW) != 0)
		roughness = 1.0 - roughness;

	float metalness = texture(u_metalnessMap, vs_vert_uv)[RMA_CHANNEL_METALNESS];
	float ao = texture(u_aoMap, vs_vert_uv)[RMA_CHANNEL_AO];

	fs_rma = vec4(0, 0, 0, 1);
	fs_rma[RMA_CHANNEL_AO] = ao;
	fs_rma[RMA_CHANNEL_ROUGHNESS] = roughness;
	fs_rma[RMA_CHANNEL_METALNESS] = metalness;
}
