#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "sh_particle_blob_shadow.gls"
#include "../fs_particle_data.gls"
#include "fs_math_scalar_field.gls"

void main()
{
	vec3 lightDirToFragment = normalize(fs_in.vert_pos_cs.xyz);
	vec3 intersection;
	float scalarFieldValue = calc_scalar_field_value(u_lightSource.data.view, fs_in.vert_pos_cs.xyz, lightDirToFragment, intersection);
	if(scalarFieldValue == 0.0)
		discard;
	vec3 fragPosWs = (inverse(u_lightSource.data.view) * vec4(intersection.xyz, 1.0)).xyz;
	gl_FragDepth = length(fragPosWs - fs_in.light_pos_ws.xyz) / fs_in.light_pos_ws.w;
}
