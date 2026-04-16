#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "/functions/fs_linearize_depth.glsl"

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, TEXTURE)) uniform sampler2D u_texture;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform PushConstants
{
	GUI_BASE_PUSH_CONSTANTS
	int alphaOnly;
	float lod;
	uint channels;

	vec4 borderSizes; // left, right, top, bottom
	vec2 uiElementSize;
	vec2 textureSize;
}
u_pushConstants;

layout(location = 0) out vec4 fs_color;

vec4 get_texture_color(vec2 uv)
{
	if(u_pushConstants.lod < 0.0)
		return texture(u_texture, uv);
	return textureLod(u_texture, uv, u_pushConstants.lod);
}

float slice_axis(float coord, float elemLo, float elemHi, float texLo, float texHi)
{
	if(coord < elemLo)
	{
		// near-border region: scale into [0 .. texLo]
		return (coord /elemLo) *texLo;
	}
	else if(coord > 1.0 -elemHi)
	{
		// far-border region: scale into [1-texHi .. 1]
		float t = (coord -(1.0 -elemHi)) /elemHi;
		return (1.0 -texHi) +t *texHi;
	}
	else
	{
		// center region: scale into [texLo .. 1-texHi]
		float t = (coord -elemLo) /(1.0 -elemLo -elemHi);
		return texLo +t *(1.0 -texLo -texHi);
	}
}

vec2 nine_slice_uv(vec2 uv)
{
	vec4 elemBorder = vec4(0, 0, 0, 0);
	vec4 texBorder = vec4(0, 0, 0, 0);
	return vec2(
		slice_axis(uv.x, elemBorder.x, elemBorder.z, texBorder.x,  texBorder.z),
		slice_axis(uv.y, elemBorder.y, elemBorder.w, texBorder.y,  texBorder.w)
	);
}

void main()
{
	vec2 uiElementSize = u_pushConstants.uiElementSize;
	vec2 texSize = u_pushConstants.textureSize;
	vec4 borderSizes = u_pushConstants.borderSizes;

	vec2 p = vs_vert_uv *uiElementSize;

	vec2 b1 = borderSizes.xz; // Left, Top
	vec2 b2 = borderSizes.yw; // Right, Bottom

	vec2 isLeftTop = step(p, b1);
	vec2 isRightBottom = step(uiElementSize -b2, p);
	vec2 isCenter = 1.0 -isLeftTop -isRightBottom;

	vec2 leftTopCoord = p;
	vec2 rightBottomCoord = texSize -(uiElementSize -p);

	vec2 quadCenterSpace = max(uiElementSize -b1 -b2, vec2(0.001));
	vec2 texCenterSpace = texSize -b1 -b2;
	
	vec2 centerCoord = b1 +((p -b1) /quadCenterSpace) *texCenterSpace;

	vec2 texPixelCoord = (isLeftTop *leftTopCoord) +(isRightBottom *rightBottomCoord) +(isCenter *centerCoord);

	vec2 uv = texPixelCoord /texSize;
	uv = nine_slice_uv(uv);

	vec4 color;
	if(u_pushConstants.alphaOnly == 1) {
		color = u_pushConstants.color;
		color.a *= get_texture_color(uv).r;
	}
	else
		color = get_texture_color(uv).rgba * u_pushConstants.color;
	fs_color = vec4(color[(u_pushConstants.channels << 24) >> 24], color[(u_pushConstants.channels << 16) >> 24], color[(u_pushConstants.channels << 8) >> 24], color[u_pushConstants.channels >> 24]);
}
