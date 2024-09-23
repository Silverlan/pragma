#ifndef F_FS_ALBEDO_MAP_GLS
#define F_FS_ALBEDO_MAP_GLS

#include "/common/material_flags.glsl"
#include "/common/alpha_mode.glsl"

vec4 get_base_color(vec4 instanceColor, vec4 albedoColor, vec4 materialColor, uint alphaMode, float alphaCutoff, uint materialFlags)
{
	vec4 baseColor = albedoColor;
	if(is_material_translucent(materialFlags)) {
		if(instanceColor.a < 1.0)
			alphaMode = ALPHA_MODE_BLEND;
		baseColor.a = apply_alpha_mode(baseColor.a * instanceColor.a * materialColor.a, alphaMode, alphaCutoff) * instanceColor.a;
	}
	else
		baseColor.a = 1.0;
	baseColor.rgb *= instanceColor.rgb * materialColor.rgb;
	return baseColor;
}

#endif
