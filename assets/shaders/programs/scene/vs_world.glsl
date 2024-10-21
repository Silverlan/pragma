#ifndef F_VS_WORLD_GLS
#define F_VS_WORLD_GLS

#include "/common/inputs/vs_skeletal_animation.glsl"
#include "/common/inputs/textures/normal_map.glsl"
#include "/common/inputs/textures/parallax_map.glsl"
#include "/common/inputs/csm.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/common/inputs/camera.glsl"
#include "/common/inputs/vs_vertex_animation.glsl"
#include "/common/inputs/vs_instanced_entity_data.glsl"
#include "/common/vertex_outputs/vs_vertex_data_locations.glsl"
#include "/common/vertex_outputs/vertex_data.glsl"
#include "/common/vertex_outputs/vs_lightmap.glsl"
#include "/common/clip_plane.glsl"
#include "/lighting/lighting.glsl"
#include "/math/math.glsl"
#include "/math/depth_bias.glsl"
#include "/functions/vector_to_depth.glsl"
#include "/util/vs_sky_camera.glsl"
#include "/programs/scene/scene_push_constants.glsl"
#include "/programs/scene/vs_config.glsl"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 in_vert_pos;
layout(location = SHADER_UV_BUFFER_LOCATION) in vec2 in_vert_uv;
layout(location = SHADER_NORMAL_BUFFER_LOCATION) in vec3 in_vert_normal;

void export_world_fragment_data(mat4 mdlMatrix, vec3 vpos, bool useNormalMap, bool useParallaxMap)
{
	mat4 V = get_view_matrix();

	vec3 vertNorm = in_vert_normal.xyz;
	vec4 vertPos = vec4(vpos, 1.0);
	mat4 mBone;
	if(CSPEC_ENABLE_ANIMATION == 1) {
		if(CSPEC_ENABLE_MORPH_TARGET_ANIMATION == 1) {
			vec3 morphTargetPos;
			vec3 morphTargetNorm;
			float wrinkleDelta;
			get_vertex_anim_data(u_pushConstants.scene.vertexAnimInfo, morphTargetPos, morphTargetNorm, wrinkleDelta);
			vertPos.xyz += morphTargetPos;
			vertNorm += morphTargetNorm;

			vertNorm = normalize(vertNorm);
			set_wrinkle_delta(wrinkleDelta);
		}
		bool extendedWeights = false;
		if(is_weighted_ext())
			extendedWeights = is_weighted_ext();
		mBone = calc_bone_matrix(is_instanced_weighted(), extendedWeights);
		vertPos = mBone * vertPos;
		vertNorm = normalize((mBone * vec4(vertNorm.xyz, 0.0)).xyz);
	}

	//mat4 MV = V *mdlMatrix;
	//mat4 MVP = u_camera.P *MV;

	//if(CSPEC_ENABLE_3D_ORIGIN == 1)
	{
		if(is_3d_sky())
			apply_sky_camera_transform(u_pushConstants.scene.drawOrigin, mdlMatrix);
	}
	vec4 vposWs = mdlMatrix * vertPos;

	mat4 P = u_camera.P;
	if(u_pushConstants.scene.depthBias.x > 0.0)
		apply_projection_depth_bias_offset(P, u_renderSettings.nearZ, u_renderSettings.farZ, u_pushConstants.scene.depthBias[0], u_pushConstants.scene.depthBias[1]);
	gl_Position = API_DEPTH_TRANSFORM(P) * V * vposWs;

	vposWs.xyz /= vposWs.w;
	vposWs.w = 1.0;

	//if(CSPEC_ENABLE_CLIPPING == 1)
	apply_clip_plane(u_pushConstants.scene.clipPlane, vposWs.xyz);
	//else
	//	gl_ClipDistance[0] = 1.0;

	set_vertex_position_ws(vposWs.xyz);
	set_model_matrix(mdlMatrix);
	set_instance_color(get_instanced_instance_color());
	//vs_out.vert_pos = vertPos;
	//vs_out.vert_pos_cs = (V *vposWs).xyz;

	set_vertex_normal(vertNorm);
	set_vertex_uv(in_vert_uv.xy);

	if(useNormalMap || useParallaxMap) {
		if(CSPEC_ENABLE_ANIMATION == 0)
			mBone = mat4(1.0);
		vec3 T = normalize((mBone * vec4(in_vert_tangent, 0.0)).xyz);
		vec3 B = normalize((mBone * vec4(in_vert_bitangent, 0.0)).xyz);
		vec3 N = vertNorm;
		set_tbn_matrix(mat3(T, B, N));
		set_vertex_normal_cs((V * mdlMatrix * vec4(vertNorm, 0)).xyz);
	}

#if ENABLE_LIGHTMAP == 1
	if(CSPEC_ENABLE_LIGHT_MAPS == 1) {
		if(is_light_map_enabled())
			set_vertex_uv_lightmap(in_uv_lightmap);
	}
#endif

	// Parallax Mapping
	/*if(use_parallax_map() == true) // We only need these if parallax mapping is enabled
	{
		vec3 tangent = normalize((mBone *vec4(-in_vert_tangent,0.0)).xyz);
		vec3 bitangent = normalize((mBone *vec4(in_vert_bitangent,0.0)).xyz);
		mat3 TBN = transpose(mat3(
			normalize((mdlMatrix *vec4(tangent,0.0)).xyz),
			normalize((mdlMatrix *vec4(bitangent,0.0)).xyz),
			normalize((mdlMatrix *vec4(normal,0.0)).xyz)
		));
		vec3 camPosTs = TBN *u_renderSettings.posCam;
		vec3 vertPosTs = TBN *vs_out.vert_pos_ws.xyz;

		vs_out.tangentData.cam_dir_to_vert_ts = camPosTs -vertPosTs;
	}*/
	//

	// export_light_fragment_data(vposWs,mBone,normal,mdlMatrix);
}

void export_world_fragment_data(vec3 vertPos, bool useNormalMap, bool useParallaxMap) { export_world_fragment_data(get_instanced_model_matrix(), vertPos, useNormalMap, useParallaxMap); }

void export_world_fragment_data(bool useNormalMap, bool useParallaxMap) { export_world_fragment_data(in_vert_pos.xyz, useNormalMap, useParallaxMap); }

#endif
