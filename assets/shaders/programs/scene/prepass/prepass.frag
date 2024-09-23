#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "prepass.glsl"
#include "prepass_vertex_data.glsl"

layout(LAYOUT_ID(TEXTURES, ALBEDO)) uniform sampler2D u_albedoMap;

layout(location = 0) out vec4 fs_normalBuffer;

layout(constant_id = SPECIALIZATION_CONSTANT_ENABLE_ALPHA_TEST) const uint CSPEC_ENABLE_ALPHA_TEST = 0;
layout(constant_id = SPECIALIZATION_CONSTANT_ENABLE_NORMAL_OUTPUT) const uint CSPEC_ENABLE_NORMAL_OUTPUT = 0;

void main()
{
	if(CSPEC_ENABLE_ALPHA_TEST == 1) {
		if(is_alpha_test_enabled()) {
			vec2 texCoords = fs_in.vert_uv; // apply_parallax(use_parallax_map(),fs_in.vert_uv);
			vec4 albedoColor = texture(u_albedoMap, texCoords);
			if(albedoColor.a < u_pushConstants.alphaCutoff)
				discard;
		}
	}
	if(CSPEC_ENABLE_NORMAL_OUTPUT == 1)
		fs_normalBuffer = vec4(fs_in.vert_normal_cs, 1.0);
}