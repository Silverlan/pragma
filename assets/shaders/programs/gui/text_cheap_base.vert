#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 in_vert_pos;
layout(location = 1) in vec2 in_vert_uv;

// Per instance (glyph)
layout(location = 2) in uint in_glyph_index;
layout(location = 3) in vec4 in_glyph_bounds;

layout(location = 0) out vec2 vs_vert_uv;

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
u_pushConstants;

#ifdef USE_VERTEX_COLOR
layout(location = 4) in vec4 in_vert_col;
layout(location = 1) out vec4 vs_vert_col;
#endif

#include "vs_shared.glsl"

void main()
{
	// Calc vertex position
	vec2 vertPos = in_vert_pos;

	vertPos = (vertPos + vec2(1.0, 1.0)) / 2.0; // Transform to range [0,1]
	vertPos = vec2(vertPos.x * (in_glyph_bounds[2] * u_pushConstants.widthScale), vertPos.y * (in_glyph_bounds[3] * u_pushConstants.heightScale) + u_pushConstants.yOffset * u_pushConstants.heightScale);
	vertPos += in_glyph_bounds.xy;

	gl_Position = get_vertex_position(vertPos);

	// Calc uv coordinates
	uint glyphIndexX = in_glyph_index % u_pushConstants.numGlyphsPerRow;
	uint glyphIndexY = in_glyph_index / u_pushConstants.numGlyphsPerRow;
	float glyphSizeW = float(u_pushConstants.maxGlyphBitmapWidth) / float(u_pushConstants.glyphMapWidth);
	float glyphSizeH = float(u_pushConstants.maxGlyphBitmapHeight) / float(u_pushConstants.glyphMapHeight);
	vec2 glyphOffset = vec2(glyphSizeW * float(glyphIndexX), glyphSizeH * float(glyphIndexY));
	float actualGlyphWidth = in_glyph_bounds[2] / float(u_pushConstants.glyphMapWidth);
	float actualGlyphHeight = in_glyph_bounds[3] / float(u_pushConstants.glyphMapHeight);
	vs_vert_uv = glyphOffset + vec2(in_vert_uv.x * actualGlyphWidth, in_vert_uv.y * actualGlyphHeight);

#ifdef USE_VERTEX_COLOR
	vs_vert_col = in_vert_col;
#endif
}
