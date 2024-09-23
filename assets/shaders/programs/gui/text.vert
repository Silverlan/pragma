#version 440

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 in_vert_pos;
layout(location = 1) in vec2 in_vert_uv;

// Per instance (glyph)
layout(location = 2) in uint in_glyph_index;
layout(location = 3) in vec4 in_glyph_bounds;

layout(location = 0) out vec2 vs_vert_uv;

layout(LAYOUT_PUSH_CONSTANTS()) uniform Font
{
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

void main()
{
	// Calc vertex position
	vec2 vertPos = in_vert_pos;

	vertPos = (vertPos + vec2(1.0, 1.0)) / 2.0; // Transform to range [0,1]
	vertPos = vec2(vertPos.x * (in_glyph_bounds[2] * u_font.widthScale), vertPos.y * (in_glyph_bounds[3] * u_font.heightScale) + u_font.yOffset * u_font.heightScale);
	vertPos += in_glyph_bounds.xy;

	gl_Position = vec4(vertPos, 0.0, 1);

	// Calc uv coordinates
	uint glyphIndexX = in_glyph_index % u_font.numGlyphsPerRow;
	uint glyphIndexY = in_glyph_index / u_font.numGlyphsPerRow;
	float glyphSizeW = float(u_font.maxGlyphBitmapWidth) / float(u_font.glyphMapWidth);
	float glyphSizeH = float(u_font.maxGlyphBitmapHeight) / float(u_font.glyphMapHeight);
	vec2 glyphOffset = vec2(glyphSizeW * float(glyphIndexX), glyphSizeH * float(glyphIndexY));
	float actualGlyphWidth = in_glyph_bounds[2] / float(u_font.glyphMapWidth);
	float actualGlyphHeight = in_glyph_bounds[3] / float(u_font.glyphMapHeight);
	vs_vert_uv = glyphOffset + vec2(in_vert_uv.x * actualGlyphWidth, in_vert_uv.y * actualGlyphHeight);
}
