#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/math/equirectangular.glsl"

struct VertexData {
	vec3 localPos;
};
layout(location = 0) in VertexData fs_in;

layout(LAYOUT_ID(TEXTURE, EQUIRECTANGULAR_TEXTURE)) uniform sampler2D u_equirectangularTexture;

layout(location = 0) out vec4 fs_color;

void main()
{
	vec2 uv = direction_to_equirectangular_uv_coordinates(normalize(fs_in.localPos));
	vec3 color = texture(u_equirectangularTexture, uv).rgb;

	fs_color = vec4(color, 1.0);
}
