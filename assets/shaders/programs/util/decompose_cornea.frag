#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "ssbump_to_normalmap.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, IRIS)) uniform sampler2D u_irisMap;
layout(LAYOUT_ID(TEXTURE, CORNEA)) uniform sampler2D u_corneaMap;

layout(location = 0) out vec4 fs_albedo;
layout(location = 1) out vec4 fs_normal;
layout(location = 2) out vec4 fs_parallax;
layout(location = 3) out vec4 fs_noise;

void main()
{
	vec4 iris = texture(u_irisMap, vs_vert_uv);
	vec4 cornea = texture(u_corneaMap, vs_vert_uv);

	float alpha = cornea.a;
	vec4 albedo = vec4(iris.rgb, alpha);
	vec3 corneaNoisiness = iris.aaa;
	vec3 normal = normalize(vec3(cornea.rg, 1.0 - (cornea.r + cornea.g)));
	normal = ss_bumpmap_sample_to_tangent_space_normal(normal);
	vec3 parallax = cornea.bbb;

	fs_albedo = albedo;
	fs_normal = vec4(normal, alpha);
	fs_parallax = vec4(parallax, alpha);
	fs_noise = vec4(corneaNoisiness, alpha);
}
