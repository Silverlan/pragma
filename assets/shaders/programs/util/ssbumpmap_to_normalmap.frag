#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "ssbump_to_normalmap.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, SS_BUMP)) uniform sampler2D u_ssBumpMap;

layout(location = 0) out vec4 fs_normal;

void main()
{
	vec4 ssBumpMapSample = texture(u_ssBumpMap, vs_vert_uv);
	vec3 n = ss_bumpmap_sample_to_tangent_space_normal(ssBumpMapSample.rgb);

	fs_normal = vec4(n, 1.0);
}
