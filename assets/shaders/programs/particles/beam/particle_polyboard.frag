#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "sh_particle_polyboard.gls"
#include "../../modules/fs_animated_texture.gls"
#include "/common/export.glsl"

layout(LAYOUT_ID(DEPTH_BUFFER, MAP)) uniform sampler2D u_smpSceneDepth;

layout(LAYOUT_PUSH_CONSTANTS()) uniform Instance
{
	layout(offset = 20) int renderFlags;
	float viewportW;
	float viewportH;
}
u_instance;

#include "../fs_particle_texture.gls"

layout(location = 0) out vec4 fs_color;
layout(location = 1) out vec4 fs_brightColor;

#if LIGHTING_ENABLED == 1
vec3 get_vertex_position_ws() { return fs_in.vert_pos_ws.xyz; }
vec3 get_vertex_position_cs() { return fs_in.vert_pos_cs; }
vec3 get_vertex_normal() { return fs_in.vert_normal; }
vec3 get_vertex_normal_cs() { return fs_in.vert_normal_cs; }
#include "../../lighting/fs_lighting.gls"
#endif

void main()
{
	vec2 coord = gl_FragCoord.xy / vec2(u_instance.viewportW, u_instance.viewportH);
	float zScene = texture(u_smpSceneDepth, coord).r;
	if(zScene < gl_FragCoord.z)
		discard;

	vec2 uv = get_particle_uv();
	fs_color = get_particle_color(uv);
	fs_color.a = get_particle_alpha(fs_color);
	fs_color *= fs_in.particle_color;
#if LIGHTING_ENABLED == 1
	if(is_particle_unlit() == false)
		fs_color.rgb = calculate_lighting(fs_color.rgb, uv, mat4(1.0), get_vertex_position_ws(), fs_in.vert_normal, false);
#endif
	fs_brightColor = fs_color;
}