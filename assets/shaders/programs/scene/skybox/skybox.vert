#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "skybox.glsl"
#include "/common/inputs/vs_instanced_entity_data.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/common/inputs/camera.glsl"
#include "/programs/scene/scene_push_constants.glsl"
#include "/math/matrix.glsl"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 in_vert_pos;

void main()
{
	mat4 MVP = API_DEPTH_TRANSFORM(u_camera.P) * get_view_matrix() * get_instanced_model_matrix();
	vec3 vtrans = in_vert_pos;
	vec4 MVP_Pos = MVP * vec4(vtrans, 1);
	gl_Position = MVP_Pos.xyww;

	vec3 camOffset = (inverse(get_instanced_model_matrix()) * vec4(u_renderSettings.posCam.xyz, 1)).xyz;
	vs_out.color = get_instanced_instance_color();
	vs_out.cubemap_dir = in_vert_pos - camOffset;
	vs_out.cubemap_dir = (get_rotation_matrix(u_pushConstants.skyAngles.xyz, u_pushConstants.skyAngles.w) * vec4(vs_out.cubemap_dir, 1.0)).xyz;
}