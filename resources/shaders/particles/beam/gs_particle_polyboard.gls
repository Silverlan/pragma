#version 440

#include "sh_particle_polyboard.gls"
#include "../../modules/sh_camera_info.gls"

layout(LAYOUT_PUSH_CONSTANTS()) uniform Instance {
	vec3 posCam;
	float radius;
	float curvature;
} u_instance;

void emit_vertex(vec3 v,vec2 uv)
{
	gl_Position = get_view_projection_matrix() *vec4(v,1.0);
	gs_out[0].particle_color = gs_in[0].particle_color;
	gs_out[0].particle_start = 0.0; // Not yet implemented
	gs_out[0].vert_uv = uv;
#if LIGHTING_ENABLED == 1
	gs_out[0].vert_pos_ws = v;
	gs_out[0].vert_pos_cs = (get_view_matrix() *vec4(v,0.0)).xyz;
	vec3 n = normalize(gs_out[0].vert_pos_ws -gs_out[0].vert_pos_cs);
	gs_out[0].vert_normal = n;
	gs_out[0].vert_normal_cs = (get_view_matrix() *vec4(n,0.0)).xyz;
#endif
	//gl_Position /= gl_Position.w;
	EmitVertex();
}

#include "gs_particle_polyboard_shared.gls"

layout(lines_adjacency) in;
layout(triangle_strip,max_vertices = PARTICLE_MAX_CURVE_VERTICES) out;

void main()
{
	process_curve_segment(u_instance.posCam.xyz);
}
