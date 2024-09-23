#ifndef F_FS_TONEMAPPING_GLS
#define F_FS_TONEMAPPING_GLS

#include "/math/math.glsl"

// Note: If any of these are changed or new ones are added, make sure to also change the Lua bindings!
#define TONE_MAPPING_NONE 0
#define TONE_MAPPING_GAMMA_CORRECTION (TONE_MAPPING_NONE + 1)
#define TONE_MAPPING_REINHARD (TONE_MAPPING_GAMMA_CORRECTION + 1)
#define TONE_MAPPING_HEJIL_RICHARD (TONE_MAPPING_REINHARD + 1)
#define TONE_MAPPING_UNCHARTED (TONE_MAPPING_HEJIL_RICHARD + 1)
#define TONE_MAPPING_ACES (TONE_MAPPING_UNCHARTED + 1)
#define TONE_MAPPING_GRAN_TURISMO (TONE_MAPPING_ACES + 1)
#define TONE_MAPPING_HDR (TONE_MAPPING_GRAN_TURISMO + 1)
#define TONE_MAPPING_COUNT (TONE_MAPPING_HDR + 1)

// Uncharted 2 tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 tone_mapping_uncharted2_impl(vec3 color)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

vec3 tone_mapping_uncharted(vec3 color)
{
	const float W = 11.2;
	color = tone_mapping_uncharted2_impl(color * 2.0);
	vec3 whiteScale = 1.0 / tone_mapping_uncharted2_impl(vec3(W));
	return linear_to_srgb(color * whiteScale);
}

// Hejl Richard tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 tone_mapping_hejil_richard(vec3 color)
{
	color = max(vec3(0.0), color - vec3(0.004));
	return (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
}

// ACES tone map
// see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 tone_mapping_aces(vec3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return linear_to_srgb(clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0));
}

vec3 tone_mapping_reinhard(vec3 color)
{
	// reinhard tone mapping
	vec3 mapped = color / (color + vec3(1.0));
	// gamma correction
	mapped = pow(mapped, vec3(1.0 / GAMMA));
	return mapped;
}

vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b)
{
	float l0 = ((P - m) * l) / a;
	float L0 = m - m / a;
	float L1 = m + (1.0 - m) / a;
	float S0 = m + l0;
	float S1 = m + a * l0;
	float C2 = (a * P) / (P - S1);
	float CP = -C2 / P;

	vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
	vec3 w2 = vec3(step(m + l0, x));
	vec3 w1 = vec3(1.0 - w0 - w2);

	vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
	vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
	vec3 L = vec3(m + a * (x - m));

	return T * w0 + L * w1 + S * w2;
}

vec3 tone_mapping_gran_turismo(vec3 x)
{
	const float P = 1.0;  // max display brightness
	const float a = 1.0;  // contrast
	const float m = 0.22; // linear section start
	const float l = 0.4;  // linear section length
	const float c = 1.33; // black
	const float b = 0.0;  // pedestal

	return linear_to_srgb(uchimura(x, P, a, m, l, c, b));
}

vec3 apply_tone_mapping(vec3 color, uint algorithm, float exposure)
{
	vec3 effColor = color * exposure;
	switch(algorithm) {
	case TONE_MAPPING_GAMMA_CORRECTION:
		return linear_to_srgb(effColor);
	case TONE_MAPPING_REINHARD:
		return tone_mapping_reinhard(effColor);
	case TONE_MAPPING_HEJIL_RICHARD:
		return tone_mapping_hejil_richard(effColor);
	case TONE_MAPPING_UNCHARTED:
		return tone_mapping_uncharted(effColor);
	case TONE_MAPPING_ACES:
		return tone_mapping_aces(effColor);
	case TONE_MAPPING_GRAN_TURISMO:
		return tone_mapping_gran_turismo(effColor);
	case TONE_MAPPING_HDR:
		return color;
	}
	return linear_to_srgb(effColor);
}

#endif
