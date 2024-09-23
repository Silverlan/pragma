#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "light_cone.glsl"

layout(location = 0) in vec3 in_vert_pos;
layout(location = 1) in vec3 in_vert_normal;

#include "/common/inputs/entity.glsl"
#include "/common/inputs/camera.glsl"

void main()
{
	vec4 vposWs = u_instance.data.M * vec4(in_vert_pos, 1);
	gl_Position = API_DEPTH_TRANSFORM(u_camera.P) * u_camera.V * vposWs;

	vposWs.xyz /= vposWs.w;
	vposWs.w = 1.0;
	vs_out.vert_pos_ws = vposWs.xyz;
	vs_out.vert_normal_cs = (u_camera.V * u_instance.data.M * vec4(in_vert_normal.xyz, 0)).xyz;
}
