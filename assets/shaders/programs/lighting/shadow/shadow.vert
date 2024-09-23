#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/common/inputs/vs_skeletal_animation.glsl"
#include "/common/inputs/vs_instanced_entity_data.glsl"
#include "/common/inputs/vs_vertex_animation.glsl"
#include "shadow.glsl"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 u_vert_pos;

void main()
{
	mat4 mBone = calc_bone_matrix(is_instanced_weighted(), is_weighted_ext());
	vec4 vpos = vec4(u_vert_pos, 1.0);
	//if(CSPEC_ENABLE_MORPH_TARGET_ANIMATION == 1)
	{
		vec3 morphTargetPos;
		vec3 morphTargetNorm;
		float wrinkleDelta;
		get_vertex_anim_data(u_pushConstants.vertexAnimInfo, morphTargetPos, morphTargetNorm, wrinkleDelta);
		vpos.xyz += morphTargetPos;
	}
	vpos = mBone * vpos;

	gl_Position = API_DEPTH_TRANSFORM(u_pushConstants.depthMVP * get_instanced_model_matrix()) * vpos;

	vec4 vposWs = get_instanced_model_matrix() * vpos;
	vposWs.xyz /= vposWs.w;
	vposWs.w = 1.0;

	vs_out.vert_pos_ws = vposWs.xyz;
	vs_out.light_pos_ws = u_pushConstants.lightPos;
}
