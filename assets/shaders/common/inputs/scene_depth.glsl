#ifndef F_SCENE_DEPTH_GLS
#define F_SCENE_DEPTH_GLS

#include "/common/depth.glsl"

#ifndef DESCRIPTOR_SET_DEPTH_MAP
#define DESCRIPTOR_SET_DEPTH_MAP DESCRIPTOR_SET_USER1
#endif

#ifndef DESCRIPTOR_SET_DEPTH_MAP_BINDING_MAP
#define DESCRIPTOR_SET_DEPTH_MAP_BINDING_MAP 0
#endif

layout(LAYOUT_ID(DEPTH_BUFFER, MAP)) uniform sampler2D u_smpSceneDepth;

float get_scene_depth(float wViewport, float hViewport)
{
	vec2 coord = gl_FragCoord.xy / vec2(wViewport, hViewport);
	return texture(u_smpSceneDepth, coord).r;
}

#endif
