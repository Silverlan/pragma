#ifndef F_VS_VERTEX_ANIMATION_GLS
#define F_VS_VERTEX_ANIMATION_GLS

#include "/math/half_float.glsl"
#include "/common/inputs/vs_vertex_animation_frame_data.glsl"

layout(std430, LAYOUT_ID(INSTANCE, VERTEX_ANIMATIONS)) buffer VertexAnimation { ivec4 verts[]; }
vertexAnimation;

void get_vertex_anim_vertex_position(uint frameOffset, out vec3 vpos, out vec3 vnorm, out float vwrinkle)
{
	uvec4 v = vertexAnimation.verts[frameOffset + SH_VERTEX_INDEX];
	vpos = vec3(float16_to_float(int(v[0] >> 16)), float16_to_float(int((v[0] << 16) >> 16)), float16_to_float(int(v[1] >> 16)));
	vwrinkle = float16_to_float(int((v[1] << 16) >> 16));
	vnorm = vec3(float16_to_float(int(v[2] >> 16)), float16_to_float(int((v[2] << 16) >> 16)), float16_to_float(int(v[3] >> 16)));
}

void get_vertex_anim_data(uint vertexAnimInfo, out vec3 vertPos, out vec3 vertNorm, out float wrinkleDelta)
{
	uint vertexAnimOffset = (vertexAnimInfo << 16) >> 16;
	uint vertexAnimCount = vertexAnimInfo >> 16;
	vertPos = vec3(0, 0, 0);
	vertNorm = vec3(0, 0, 0);
	wrinkleDelta = 0.0;
	for(uint i = 0; i < vertexAnimCount; ++i) {
		uint offset = vertexAnimOffset + i;
		VertexAnimationFrameData frameData = vaFrameData.data[offset];

		vec3 vpos0;
		vec3 vnorm0;
		float vwrinkle0;
		get_vertex_anim_vertex_position(frameData.srcFrameOffset, vpos0, vnorm0, vwrinkle0);

		vec3 vpos1;
		vec3 vnorm1;
		float vwrinkle1;
		get_vertex_anim_vertex_position(frameData.srcFrameOffset, vpos1, vnorm1, vwrinkle1);

		wrinkleDelta += vwrinkle0 * frameData.blend;
		vertPos.xyz += vpos0 * frameData.blend; //v0 +(v1 -v0) *frameData.blend;
		vertNorm += vnorm0 * frameData.blend;
	}
}

#endif
