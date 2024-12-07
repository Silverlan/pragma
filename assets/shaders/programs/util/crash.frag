#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(location = 0) out float fs_alpha;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	float offset;
}
u_pushConstants;

void main()
{
	fs_alpha = 1.0;
	while (fs_alpha > 0.0)
	{
		fs_alpha -= u_pushConstants.offset;
	}
	fs_alpha *= 0.1 * fs_alpha;
}
