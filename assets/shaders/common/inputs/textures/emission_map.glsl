#ifndef F_FS_GLOWMAP_GLS
#define F_FS_GLOWMAP_GLS

#include "/common/material_flags.glsl"
#include "/common/fs_tonemapping.glsl"

#ifdef MATERIAL_EMISSION_MAP_ENABLED
layout(LAYOUT_ID(MATERIAL, EMISSION_MAP)) uniform sampler2D u_glowMap;
vec4 get_glow_map_color(vec2 uv) { return texture(u_glowMap, uv).rgba; }
#endif

#define GLOW_MODE_ADDITIVE 0
#define GLOW_MODE_OVERLAY 1
#define GLOW_MODE_MIX 2
#define GLOW_MODE_MODULATED 3

vec4 add_glow_color(vec4 col, vec2 texCoords, vec4 glowColor, vec4 baseColor, uint glowMode)
{
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	vec3 colGlow = glowColor.rgb * glowColor.a; // *get_glow_scale();
	switch(glowMode) {
	case GLOW_MODE_ADDITIVE:
		col.rgb += colGlow;
		break;
	case GLOW_MODE_OVERLAY:
		{
			for(int i = 0; i < 3; i++)
				col[i] += max(colGlow[i] - col[i], 0.0);
			break;
		}
	case GLOW_MODE_MIX:
		{
			for(int i = 0; i < 3; i++) {
				float scale = colGlow[i] / 1.0;
				col[i] = col[i] * (1.0 - scale) + colGlow[i] * scale;
			}
			break;
		}
	case GLOW_MODE_MODULATED:
		{
			col.rgb = max(col.rgb, baseColor.rgb * colGlow.rgb);
			break;
		}
	}
#endif
	return col;
}
vec4 add_glow_color(uint materialFlags, vec4 col, vec2 texCoords, vec4 glowColor, vec4 baseColor)
{
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	if((materialFlags & FMAT_FLAGS_GLOW_MODE1) != 0)
		return add_glow_color(col, texCoords, glowColor, baseColor, GLOW_MODE_ADDITIVE);
	else if((materialFlags & FMAT_FLAGS_GLOW_MODE2) != 0)
		return add_glow_color(col, texCoords, glowColor, baseColor, GLOW_MODE_OVERLAY);
	else if((materialFlags & FMAT_FLAGS_GLOW_MODE3) != 0)
		return add_glow_color(col, texCoords, glowColor, baseColor, GLOW_MODE_MIX);
	else
		return add_glow_color(col, texCoords, glowColor, baseColor, GLOW_MODE_MODULATED);
#endif
	return col;
}

vec4 add_glow_map_color(uint materialFlags, vec4 col, vec2 texCoords, vec4 baseColor)
{
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	if(use_glow_map(materialFlags))
		return add_glow_color(materialFlags, col, texCoords, texture(u_glowMap, texCoords), baseColor);
#endif
	return col;
}

vec4 get_emission_color(vec4 color, vec4 baseColor, vec2 texCoords, vec3 materialEmissionFactor, uint materialFlags)
{
	vec4 result = color;
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	if(use_glow_map(materialFlags)) {
		vec4 emissiveColor = texture(u_glowMap, texCoords);
		emissiveColor.rgb *= materialEmissionFactor.rgb;
		emissiveColor.rgb *= emissiveColor.a * 15;
		result = add_glow_color(result, texCoords, emissiveColor, baseColor, materialFlags);
	}
#endif
	return result;
}

#endif
