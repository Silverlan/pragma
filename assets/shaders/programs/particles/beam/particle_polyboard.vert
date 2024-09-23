#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SHADER_VERTEX_BUFFER_LOCATION 0
#define SHADER_COLOR_BUFFER_LOCATION 1

#include "sh_particle_polyboard.gls"
#include "../../modules/sh_camera_info.gls"
#include "../../modules/sh_rendersettings.gls"
//#include "../../modules/vs_light.gls"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 in_vert_pos;
layout(location = SHADER_COLOR_BUFFER_LOCATION) in vec4 in_color;

void main()
{
	gl_Position = vec4(in_vert_pos, 1.0);
	vs_out.particle_color = in_color;
#if LIGHTING_ENABLED == 1
	vs_out.vert_pos_ws = in_vert_pos;
	vs_out.vert_pos_cs = (get_view_matrix() * vec4(in_vert_pos, 0.0)).xyz;
	vec3 n = normalize(vs_out.vert_pos_ws - vs_out.vert_pos_cs);
	vs_out.vert_normal = n;
	vs_out.vert_normal_cs = (get_view_matrix() * vec4(n, 0.0)).xyz;

	//export_light_fragment_data(get_model_matrix() *vec4(in_vert_pos,1.0));
#endif
}
