#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "prepass.glsl"
#include "/modules/vs_weighted.gls"
#include "/modules/vs_vertex_animation.gls"
#include "/common/inputs/material.glsl"
#include "/modules/sh_clip_plane.gls"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 in_vert_pos;

void main()
{
	vec3 vertPos = in_vert_pos.xyz;
	uint vertexAnimOffset = (u_pushConstants.scene.vertexAnimInfo << 16) >> 16;
	uint vertexAnimCount = u_pushConstants.scene.vertexAnimInfo >> 16;
	for(uint i = 0; i < vertexAnimCount; ++i) {
		uint offset = vertexAnimOffset + i;
		VertexAnimationFrameData frameData = vaFrameData.data[offset];

		vec3 vpos0;
		vec3 vnorm0;
		float vwrinkle0;
		get_vertex_anim_vertex_position(frameData.srcFrameOffset, vpos0, vnorm0, vwrinkle0);

		vec3 vpos1;
		vec3 vnorm1;
		float vwrinkle1;
		get_vertex_anim_vertex_position(frameData.srcFrameOffset, vpos1, vnorm1, vwrinkle1);

		vertPos += vpos0 * frameData.blend; //v0 +(v1 -v0) *frameData.blend;
	}

	mat4 mBone = calc_bone_matrix(is_weighted(), is_weighted_ext());
	vec4 weightedVertPos = mBone * vec4(vertPos, 1.0);

	mat4 mdlMatrix = get_model_matrix();
	vec4 vposWs = mdlMatrix * weightedVertPos;
	if(is_3d_sky()) {
		vposWs.xyz -= u_pushConstants.scene.drawOrigin.xyz;
		vposWs.xyz *= u_pushConstants.scene.drawOrigin.w;
	}

	mat4 V = get_view_matrix();
	gl_Position = API_COORD_TRANSFORM(u_camera.P) * V * vposWs;

	vposWs.xyz /= vposWs.w;
	vposWs.w = 1.0;

	apply_clip_plane(u_pushConstants.scene.clipPlane, vposWs.xyz);
}
