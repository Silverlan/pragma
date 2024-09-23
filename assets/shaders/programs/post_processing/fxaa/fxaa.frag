#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define FXAA_PC 1
#define FXAA_QUALITY__PRESET 12
#define FXAA_GLSL_130 1
// #define FXAA_GREEN_AS_LUMA 1
#include "fxaa3_11.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURES, SCENE)) uniform sampler2D u_texture;
layout(LAYOUT_ID(TEXTURES, PRE_TONEMAPPING)) uniform sampler2D u_alphaTexture;

layout(location = 0) out vec4 fs_color;

layout(LAYOUT_PUSH_CONSTANTS()) uniform FXAAConstants
{
	float subPixelAliasingRemoval;
	float edgeThreshold;
	float minEdgeThreshold;
}
u_fxaaSettings;

void main()
{
	ivec2 resolution = textureSize(u_texture, 0);
	fs_color = FxaaPixelShader(vs_vert_uv,                        /* pos */
	  vec4(0, 0, 0, 0),                                           /* unused */
	  u_texture,                                                  /* tex */
	  u_texture,                                                  /* unused */
	  u_texture,                                                  /* unused */
	  vec2(1.0 / float(resolution.x), 1.0 / float(resolution.y)), /* fxaaQualityRcpFrame */
	  vec4(0, 0, 0, 0),                                           /* unused */
	  vec4(0, 0, 0, 0),                                           /* unused */
	  vec4(0, 0, 0, 0),                                           /* unused */
	  u_fxaaSettings.subPixelAliasingRemoval,                     /* fxaaQualitySubpix */
	  u_fxaaSettings.edgeThreshold,                               /* fxaaQualityEdgeThreshold*/
	  u_fxaaSettings.minEdgeThreshold,                            /* fxaaQualityEdgeThresholdMin */
	  0.0,                                                        /* unused */
	  0.0,                                                        /* unused */
	  0.0,                                                        /* unused */
	  vec4(0, 0, 0, 0)                                            /* unused */
	);
	fs_color.a = texture(u_alphaTexture, vs_vert_uv).a;
}
