#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SHADER_BONE_WEIGHT_ID_LOCATION 1
#define SHADER_BONE_WEIGHT_LOCATION (SHADER_BONE_WEIGHT_ID_LOCATION + 1)

#define SHADER_BONE_WEIGHT_EXT_ID_LOCATION (SHADER_BONE_WEIGHT_LOCATION + 1)
#define SHADER_BONE_WEIGHT_EXT_LOCATION (SHADER_BONE_WEIGHT_EXT_ID_LOCATION + 1)

#define SHADER_VERTEX_BUFFER_LOCATION (SHADER_BONE_WEIGHT_EXT_LOCATION + 1)

#include "glow.glsl"
#include "/common/inputs/vs_skeletal_animation.glsl"
#include "/common/inputs/camera.glsl"
#include "/common/inputs/entity.glsl"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 in_vert_pos;
layout(location = SHADER_UV_BUFFER_LOCATION) in vec2 in_vert_uv;

void main()
{
	vec4 vpos = get_weighted_vertex_position(is_weighted(), vec4(in_vert_pos, 1.0));
	mat4 MVP = get_view_projection_matrix() * get_model_matrix();
	gl_Position = MVP * vpos;
	vs_out.vert_uv = in_vert_uv;
}
