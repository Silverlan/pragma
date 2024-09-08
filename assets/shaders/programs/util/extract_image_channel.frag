#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "ssbump_to_normalmap.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, IMAGE)) uniform sampler2D u_imageMap;

layout(location = 0) out vec4 fs_color;

#define CHANNEL_SRC_RED 0
#define CHANNEL_SRC_GREEN 1
#define CHANNEL_SRC_BLUE 2
#define CHANNEL_SRC_ALPHA 3
#define CHANNEL_ZERO 4
#define CHANNEL_ONE 5

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	uint channelDstRed;
	uint channelDstGreen;
	uint channelDstBlue;
	uint channelDstAlpha;
}
u_pushConstants;

float get_channel_value(vec4 srcColor, uint channelMode)
{
	switch(channelMode) {
	case CHANNEL_ZERO:
		return 0.0;
	case CHANNEL_ONE:
		return 1.0;
	default:
		return srcColor[channelMode];
	}
	return 0.0;
}

void main()
{
	vec4 color = texture(u_imageMap, vs_vert_uv);

	fs_color = vec4(get_channel_value(color, u_pushConstants.channelDstRed), get_channel_value(color, u_pushConstants.channelDstGreen), get_channel_value(color, u_pushConstants.channelDstBlue), get_channel_value(color, u_pushConstants.channelDstAlpha));
}
