#version 440

#include "glow.glsl"

vec4 get_glow_map_color() { return texture(u_glowMap, fs_in.vert_uv).rgba; }

layout(LAYOUT_PUSH_CONSTANTS()) uniform pushConstants { float glowScale; }
u_pushConstants;

layout(location = 0) out vec4 fs_color;

void main()
{
	fs_color = get_glow_map_color();
	fs_color.rgb *= u_pushConstants.glowScale;
}
