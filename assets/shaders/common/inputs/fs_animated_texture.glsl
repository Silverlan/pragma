#ifndef F_ANIMATED_TEXTURE_NEW_GLS
#define F_ANIMATED_TEXTURE_NEW_GLS

#include "/common/inputs/time.glsl"

struct TextureAnimationFrame {
	vec2 uvStart;
	vec2 uvEnd;
};

layout(std430, LAYOUT_ID(ANIMATION,ANIMATION_DATA)) buffer TextureAnimation { TextureAnimationFrame frames[]; }
u_animation;

#endif
