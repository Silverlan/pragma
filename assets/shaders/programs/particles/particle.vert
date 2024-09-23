#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SHADER_POS_BUFFER_LOCATION 0
#define SHADER_RADIUS_BUFFER_LOCATION 1
#define SHADER_PREVPOS_BUFFER_LOCATION 2
#define SHADER_AGE_BUFFER_LOCATION 3
#define SHADER_COLOR_BUFFER_LOCATION 4
#define SHADER_ROTATION_BUFFER_LOCATION 5
#define SHADER_LENGTH_YAW_BUFFER_LOCATION 6
#define SHADER_ANIMATION_FRAME_INDICES_LOCATION 7
#define SHADER_ANIMATION_FRAME_INTERP_FACTOR_LOCATION 8

#include "particle_generic.glsl"
#include "vs_particle_vertex.glsl"
#include "particle_vertex_data.glsl"
#include "particle.glsl"
#include "/common/inputs/camera.glsl"

void main()
{
	vec3 vertexPosition_worldspace = get_particle_vertex_position(u_instance.camPos);
	gl_Position = get_view_projection_matrix() * vec4(vertexPosition_worldspace, 1.0);

	vs_out.vert_uv = get_vertex_quad_pos() + vec2(0.5, 0.5);
	vs_out.particle_color = in_color;
	// vs_out.particle_start = in_animationStart;
	vs_out.animationFrameIndices = in_animFrameIndices;
	vs_out.animationFrameInterpFactor = in_animInterpFactor;

#if LIGHTING_ENABLED == 1
	vs_out.vert_pos_ws.xyz = vertexPosition_worldspace;
	vs_out.vert_pos_cs = (get_view_matrix() * vec4(vertexPosition_worldspace, 0.0)).xyz;
	vec3 n = normalize(vs_out.vert_pos_ws.xyz - vs_out.vert_pos_cs);
	vs_out.vert_normal = n;
	vs_out.vert_normal_cs = (get_view_matrix() * vec4(n, 0.0)).xyz;

	//export_light_fragment_data(get_model_matrix() *vec4(vertexPosition_worldspace,1.0));
#endif
}
