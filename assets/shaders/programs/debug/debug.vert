#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define USE_VERTEX_COLOR_BUFFER 0

#define SHADER_VERTEX_BUFFER_LOCATION 0
#define SHADER_VERTEX_COLOR_LOCATION 1

#include "debug.glsl"

layout(location = SHADER_VERTEX_BUFFER_LOCATION) in vec3 in_vert_pos;

#if USE_VERTEX_COLOR_BUFFER == 1

layout(location = SHADER_VERTEX_COLOR_LOCATION) in vec4 in_vert_col;

#endif

layout(LAYOUT_PUSH_CONSTANTS()) uniform InstanceData
{
	mat4 MVP;
	vec4 color;
}
u_instanceData;

void main()
{
	gl_Position = API_DEPTH_TRANSFORM(u_instanceData.MVP) * vec4(in_vert_pos, 1);
	gl_PointSize = 4.0;
#if USE_VERTEX_COLOR_BUFFER == 1
	vs_out.frag_col = in_vert_col;
#else
	vs_out.frag_col = u_instanceData.color;
#endif
}