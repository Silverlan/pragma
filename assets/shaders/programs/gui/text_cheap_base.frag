#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 vs_vert_uv;

layout(LAYOUT_ID(TEXTURE, GLYPH_MAP)) uniform sampler2D u_texture;

layout(location = 0) out vec4 fs_color;

#include "base_push_constants.glsl"

layout(LAYOUT_PUSH_CONSTANTS()) uniform Font
{
	GUI_BASE_PUSH_CONSTANTS
	float widthScale;
	float heightScale;
	uint glyphMapWidth;
	uint glyphMapHeight;
	uint maxGlyphBitmapWidth;
	uint maxGlyphBitmapHeight;
	uint yOffset;
	uint numGlyphsPerRow;
}
u_font;

#ifdef USE_VERTEX_COLOR
layout(location = 1) in vec4 vs_vert_col;
#endif

void main()
{
	float r = texture(u_texture, vs_vert_uv).r;
	if(r == 0.0)
		discard;
#ifdef USE_VERTEX_COLOR
	fs_color = vec4(vs_vert_col.rgb, r.r);
#else
	fs_color = vec4(u_font.color.rgb, r.r);
#endif
}
