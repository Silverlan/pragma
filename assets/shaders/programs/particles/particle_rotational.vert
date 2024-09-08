#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SHADER_VERTEX_BUFFER_LOCATION 0
#define SHADER_XYZS_BUFFER_LOCATION 1
#define SHADER_COLOR_BUFFER_LOCATION 2
#define SHADER_ROTATION_BUFFER_LOCATION 3
#define SHADER_LENGTH_BUFFER_LOCATION 4
#define SHADER_ANIMATION_START_LOCATION 5
#define SHADER_WORLD_ROTATION_LOCATION 6

#include "sh_particle_mode.gls"
#include "/common/export.glsl"
#include "sh_particle_generic.gls"
#include "../modules/sh_matrix.gls"
#include "/common/inputs/camera.glsl"
#include "../modules/sh_quaternion.gls"
#include "sh_particle.gls"

#if LIGHTING_ENABLED == 1
//#include "../modules/vs_light.gls"
#endif

layout(location = SHADER_WORLD_ROTATION_LOCATION) in vec4 in_world_rot;

void main()
{
	vec3 particleCenterWs = in_xyzs.xyz;
	vec2 vsize = vec2(get_particle_extent(in_xyzs.w), get_particle_extent(in_length));
	vec3 squareVert = vec3(in_squareVertices, 0.0);

	mat4 v = get_view_matrix();
	vec3 vCorner;
	switch(u_instance.orientation) {
	case PARTICLE_ORIENTATION_TYPE_VELOCITY:
		{
			vec3 dir = vec3(in_world_rot.x, in_world_rot.y, in_world_rot.z); // in_world_rot is up vector
			vec3 right = cross(normalize(particleCenterWs - u_instance.camPos), dir);
			vec3 up = -dir;
			squareVert = (get_rotation_matrix(normalize(vec3(0, 0, 1)), radians(in_rotation)) * vec4(squareVert.xyz, 1)).xyz;

			vCorner = right * squareVert.x * vsize.x + up * squareVert.y * vsize.y;
			break;
		}
	case PARTICLE_ORIENTATION_TYPE_ALIGNED:
		{
			// Particles always face camera, but keep their world rotation
			vec3 right = u_instance.camRight_ws;
			vec3 up = u_instance.camUp_ws;

			squareVert = rotate_vector(squareVert, in_world_rot);
			vCorner = right * squareVert.x * vsize.x + up * squareVert.y * vsize.y;
			break;
		}
	//case PARTICLE_ORIENTATION_TYPE_BILLBOARD: // Deprecated?
	//	vCorner = get_corner_particle_vertex_position(u_instance.camPos);
	//	vCorner = rotate_vector(vCorner,in_world_rot);
	//	break;
	default:
		vCorner = vec3(1, 0, 0) * squareVert.x * vsize.x + vec3(0, 0, 1) * squareVert.y * vsize.y;
		vCorner = rotate_vector(vCorner, in_world_rot);
		break;
	}
	vec3 vertexPosition_worldspace = particleCenterWs + vCorner;

	gl_Position = get_view_projection_matrix() * vec4(vertexPosition_worldspace, 1.0);

	vs_out.vert_uv = in_squareVertices + vec2(0.5, 0.5);
	vs_out.particle_color = in_color;

	vs_out.particle_start = in_animationStart;

#if LIGHTING_ENABLED == 1
	vs_out.vert_pos_ws.xyz = vertexPosition_worldspace;
	vs_out.vert_pos_cs = (get_view_matrix() * vec4(vertexPosition_worldspace, 0.0)).xyz;
	vec3 n = normalize(vs_out.vert_pos_ws.xyz - vs_out.vert_pos_cs);
	vs_out.vert_normal = n;
	vs_out.vert_normal_cs = (get_view_matrix() * vec4(n, 0.0)).xyz;

	//export_light_fragment_data(get_model_matrix() *vec4(vertexPosition_worldspace,1.0));
#endif
}
