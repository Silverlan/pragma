#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/math/math.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_ssao;

layout(location = 0) out float fs_ssao;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(u_ssao, 0));
	float result = 0.0;
	for(int x = -2; x < 2; ++x) {
		for(int y = -2; y < 2; ++y) {
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(u_ssao, vs_vert_uv + offset).r;
		}
	}
	fs_ssao = result / pow2(4.0);
}
