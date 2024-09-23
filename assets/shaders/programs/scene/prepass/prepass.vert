#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "prepass.glsl"
#include "prepass_vertex_data.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/common/inputs/vs_skeletal_animation.glsl"
#include "/common/inputs/vs_vertex_animation.glsl"
#include "/common/inputs/vs_instanced_entity_data.glsl"
#include "/util/vs_sky_camera.glsl"
#include "/common/clip_plane.glsl"
#include "/math/depth_bias.glsl"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 in_vert_pos;
layout(location = SHADER_UV_BUFFER_LOCATION) in vec2 in_vert_uv;
layout(location = SHADER_NORMAL_BUFFER_LOCATION) in vec3 in_vert_normal;

layout(constant_id = SPECIALIZATION_CONSTANT_ENABLE_ANIMATION) const uint CSPEC_ENABLE_ANIMATION = 1;
layout(constant_id = SPECIALIZATION_CONSTANT_ENABLE_MORPH_TARGET_ANIMATION) const uint CSPEC_ENABLE_MORPH_TARGET_ANIMATION = 1;

void main()
{
	mat4 mdlMatrix = get_instanced_model_matrix();
	mat4 V = get_view_matrix();

	vec4 vertPos = vec4(in_vert_pos.xyz, 1.0);
	vec3 normal = in_vert_normal.xyz;
	if(CSPEC_ENABLE_ANIMATION == 1) {
		if(CSPEC_ENABLE_MORPH_TARGET_ANIMATION == 1) {
			vec3 morphTargetPos;
			vec3 morphTargetNorm;
			float wrinkleDelta;
			get_vertex_anim_data(u_pushConstants.scene.vertexAnimInfo, morphTargetPos, morphTargetNorm, wrinkleDelta);
			vertPos.xyz += morphTargetPos;
		}
		mat4 mBone = calc_bone_matrix(is_instanced_weighted(), is_weighted_ext());
		vertPos = mBone * vertPos;
		normal = normalize((mBone * vec4(normal.xyz, 0.0)).xyz);
	}

	if(is_3d_sky())
		apply_sky_camera_transform(u_pushConstants.scene.drawOrigin, mdlMatrix);
	vec4 vposWs = mdlMatrix * vertPos;

	mat4 P = u_camera.P;
	if(u_pushConstants.scene.depthBias.x > 0.0)
		apply_projection_depth_bias_offset(P, u_renderSettings.nearZ, u_renderSettings.farZ, u_pushConstants.scene.depthBias[0], u_pushConstants.scene.depthBias[1]);
	gl_Position = API_DEPTH_TRANSFORM(P) * V * vposWs;

	vposWs.xyz /= vposWs.w;
	vposWs.w = 1.0;

	apply_clip_plane(u_pushConstants.scene.clipPlane, vposWs.xyz);

	vs_out.vert_normal_cs = (V * mdlMatrix * vec4(normal, 0)).xyz;
	vs_out.vert_uv = in_vert_uv.xy;
}
