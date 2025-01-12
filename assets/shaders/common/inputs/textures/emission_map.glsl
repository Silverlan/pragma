#ifndef F_FS_GLOWMAP_GLS
#define F_FS_GLOWMAP_GLS

#include "/common/material_flags.glsl"
#include "/common/fs_tonemapping.glsl"
#include "/common/emission.glsl"

#ifdef MATERIAL_EMISSION_MAP_ENABLED
layout(LAYOUT_ID(MATERIAL, EMISSION_MAP)) uniform sampler2D u_glowMap;
vec4 get_glow_map_color(vec2 uv) { return texture(u_glowMap, uv).rgba; }
#endif

uint get_glow_mode(uint materialFlags)
{
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	if((materialFlags & FMAT_FLAGS_GLOW_MODE1) != 0)
		return EMISSION_MODE_ADDITIVE;
	else if((materialFlags & FMAT_FLAGS_GLOW_MODE2) != 0)
		return EMISSION_MODE_HIGHLIGHT_BOOST;
	else if((materialFlags & FMAT_FLAGS_GLOW_MODE3) != 0)
		return EMISSION_MODE_BLEND;
#endif
	return EMISSION_MODE_MODULATE;
}

vec4 apply_emission_color(uint materialFlags, vec4 col, vec4 glowColor, vec4 baseColor)
{
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	if((materialFlags & FMAT_FLAGS_GLOW_MODE1) != 0)
		return apply_emission_color(col, glowColor, baseColor, EMISSION_MODE_ADDITIVE);
	else if((materialFlags & FMAT_FLAGS_GLOW_MODE2) != 0)
		return apply_emission_color(col, glowColor, baseColor, EMISSION_MODE_HIGHLIGHT_BOOST);
	else if((materialFlags & FMAT_FLAGS_GLOW_MODE3) != 0)
		return apply_emission_color(col, glowColor, baseColor, EMISSION_MODE_BLEND);
	else
		return apply_emission_color(col, glowColor, baseColor, EMISSION_MODE_MODULATE);
#endif
	return col;
}

vec4 add_glow_map_color(uint materialFlags, vec4 col, vec2 texCoords, vec4 baseColor)
{
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	if(use_glow_map(materialFlags))
		return apply_emission_color(materialFlags, col, texture(u_glowMap, texCoords), baseColor);
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
		result = apply_emission_color(result, emissiveColor, baseColor, get_glow_mode(materialFlags));
	}
#endif
	return result;
}

#endif
