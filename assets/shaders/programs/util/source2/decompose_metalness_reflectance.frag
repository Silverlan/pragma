#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/rma.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, METALNESS_REFLECTANCE)) uniform sampler2D u_metalnessReflectanceMap;

layout(location = 0) out vec4 fs_rma;

void main()
{
	vec4 metalnessReflectance = texture(u_metalnessReflectanceMap, vs_vert_uv);

	float ao = 1.0;
	float metalness = metalnessReflectance.r;
	float roughness = metalnessReflectance.g;

	fs_rma = vec4(0, 0, 0, 1);
	fs_rma[RMA_CHANNEL_AO] = ao;
	fs_rma[RMA_CHANNEL_ROUGHNESS] = roughness;
	fs_rma[RMA_CHANNEL_METALNESS] = metalness;
}
