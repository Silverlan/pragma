#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 in_vert_pos;

struct VertexData {
	vec3 localPos;
};
layout(location = 0) out VertexData vs_out;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	mat4 projection;
	mat4 view;
}
u_pushConstants;

void main()
{
	vs_out.localPos = in_vert_pos;
	gl_Position = u_pushConstants.projection * u_pushConstants.view * vec4(vs_out.localPos, 1.0);
}
