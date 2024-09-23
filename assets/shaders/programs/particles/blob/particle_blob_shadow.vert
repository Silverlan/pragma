#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SHADER_VERTEX_BUFFER_LOCATION 0
#define SHADER_XYZS_BUFFER_LOCATION (SHADER_VERTEX_BUFFER_LOCATION + 1)
#define SHADER_BLOB_NEIGHBOR_INDEX_LOCATION (SHADER_XYZS_BUFFER_LOCATION + 1)

layout(location = SHADER_BLOB_NEIGHBOR_INDEX_LOCATION) in uvec4 in_blobNeighbors;

#define ENABLE_PARTICLE_ROTATION 0
#define ENABLE_PARTICLE_ORIENTATION 0

#include "sh_particle_blob_shadow.gls"
#include "../vs_particle_vertex.gls"

void main()
{
	vec3 vertPos = get_particle_vertex_position(u_instance.lightPos.xyz);
	gl_Position = u_instance.VP * vec4(vertPos, 1.0);

	vs_out.vert_pos_ws = vertPos;
	vs_out.light_pos_ws = u_instance.lightPos;
	vs_out.vert_pos_cs = (u_lightSource.data.view * vec4(vertPos, 1.0)).xyz;
	vs_out.blobNeighbors = in_blobNeighbors;
}
