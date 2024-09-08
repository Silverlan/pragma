#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SHADER_UNIFORM_TEXTURE_SET 0
#define SHADER_UNIFORM_TEXTURE_BINDING 0

#define SHADER_UNIFORM_DEPTH_BUFFER_SET (SHADER_UNIFORM_TEXTURE_SET + 1)
#define SHADER_UNIFORM_DEPTH_BUFFER_BINDING 0

#define SHADER_UNIFORM_RENDER_SETTINGS_SET (SHADER_UNIFORM_DEPTH_BUFFER_SET + 1)
#define SHADER_UNIFORM_RENDER_SETTINGS_BINDING 1

#define SHADER_UNIFORM_FOG_SET (SHADER_UNIFORM_RENDER_SETTINGS_SET + 1)
#define SHADER_UNIFORM_FOG_BINDING 0

#include "/common/inputs/fs_fog.glsl"
#include "/math/math.glsl"
#include "/common/inputs/render_settings.glsl"
#include "/functions/position_from_depth.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;
layout(LAYOUT_ID(DEPTH_BUFFER, MAP)) uniform sampler2D u_depthBuffer;

layout(location = 0) out vec4 fs_color;

void main()
{
	fs_color = texture(u_texture, vs_vert_uv);

	float d = texture(u_depthBuffer, vs_vert_uv).r;
	float depthDist = get_depth_distance(d, u_renderSettings.nearZ, u_renderSettings.farZ);
	fs_color.rgb = calculate_fog(fs_color.rgb, depthDist);
}
