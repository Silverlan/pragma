#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	uint redChannel;
	uint greenChannel;
	uint blueChannel;
	uint alphaChannel;
}
u_pushConstants;

layout(LAYOUT_ID(TEXTURES, CHANNEL_R)) uniform sampler2D u_channelR;
layout(LAYOUT_ID(TEXTURES, CHANNEL_G)) uniform sampler2D u_channelG;
layout(LAYOUT_ID(TEXTURES, CHANNEL_B)) uniform sampler2D u_channelB;
layout(LAYOUT_ID(TEXTURES, CHANNEL_A)) uniform sampler2D u_channelA;

layout(location = 0) out vec4 fs_output;

void main()
{
	vec4 col0 = texture(u_channelR, vs_vert_uv);
	vec4 col1 = texture(u_channelG, vs_vert_uv);
	vec4 col2 = texture(u_channelB, vs_vert_uv);
	vec4 col3 = texture(u_channelA, vs_vert_uv);
	fs_output = vec4(col0[u_pushConstants.redChannel], col1[u_pushConstants.greenChannel], col2[u_pushConstants.blueChannel], col3[u_pushConstants.alphaChannel]);
}
