#ifndef F_SH_EMISSION_GLS
#define F_SH_EMISSION_GLS

#define EMISSION_MODE_ADDITIVE 0
#define EMISSION_MODE_HIGHLIGHT_BOOST 1
#define EMISSION_MODE_BLEND 2
#define EMISSION_MODE_MODULATE 3
#define EMISSION_MODE_SCREEN 4
#define EMISSION_MODE_SOFT_LIGHT 5
#define EMISSION_MODE_MULTIPLY 6

vec4 apply_emission_color(vec4 col, vec4 glowColor, vec4 baseColor, uint emissionMode)
{
#ifdef MATERIAL_EMISSION_MAP_ENABLED
	vec3 colGlow = glowColor.rgb * glowColor.a; // *get_glow_scale();
	switch(emissionMode) {
	case EMISSION_MODE_ADDITIVE:
		col.rgb += colGlow;
		break;
	case EMISSION_MODE_HIGHLIGHT_BOOST:
		{
			for(int i = 0; i < 3; i++)
				col[i] += max(colGlow[i] - col[i], 0.0);
			break;
		}
	case EMISSION_MODE_BLEND:
		{
			for(int i = 0; i < 3; i++) {
				float scale = colGlow[i] / 1.0;
				col[i] = col[i] * (1.0 - scale) + colGlow[i] * scale;
			}
			break;
		}
	case EMISSION_MODE_MODULATE:
		{
			col.rgb = max(col.rgb, baseColor.rgb * colGlow.rgb);
			break;
		}
	case EMISSION_MODE_SCREEN:
		{
			col.rgb = 1.0 - (1.0 - col.rgb) * (1.0 - colGlow.rgb);
			break;
		}
	case EMISSION_MODE_SOFT_LIGHT:
		{
			col.rgb = (1.0 - colGlow.rgb) * col.rgb * col.rgb + colGlow.rgb * (1.0 - (1.0 - col.rgb) * (1.0 - col.rgb));
			break;
		}
	case EMISSION_MODE_MULTIPLY:
		{
			col.rgb *= colGlow.rgb;
			break;
		}
	}
#endif
	return col;
}

#endif
