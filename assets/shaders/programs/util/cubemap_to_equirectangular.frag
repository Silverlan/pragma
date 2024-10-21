#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/math/math.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform samplerCube u_cubemap;

layout(location = 0) out vec4 fs_color;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants { float xFactor; }
u_pushConstants;

// Source: https://stackoverflow.com/a/35050222/2482983
void main()
{
	vec2 uv = vs_vert_uv * 2.0 - vec2(1.0, 1.0);

	// Convert to (lat, lon) angle
	vec2 a = (uv * vec2(M_PI * u_pushConstants.xFactor, M_PI_2)) - vec2(M_PI / 2, 0);
	// Convert to cartesian coordinates
	vec2 c = cos(a);
	vec2 s = sin(a);
	fs_color = texture(u_cubemap, vec3(-s.x * c.y, -s.y, c.x * c.y));
}
