#ifndef F_FS_SHADOW_POINT_GLS
#define F_FS_SHADOW_POINT_GLS

#include "/math/math.glsl"
#include "shadow_settings.glsl"

#if SHADOW_PCF_ENABLED == 1
float get_shadow_value(shadowCubeMapSamplerType shadowMap, vec3 c, float fragDepth) { return texture(shadowMap, vec4(c, fragDepth)).r; }
#else
float get_shadow_value(shadowCubeMapSamplerType shadowMap, vec3 c, float fragDepth)
{
	float d = texture(shadowMap, c).r;
	return (d < fragDepth) ? 0.0 : 1.0;
}
#endif

float exp_scale_factor(float v, float e) // Grows more quickly the closer it comes to 1
{
	return 1.0 - pow((1.0 - v), e);
}

float calculate_point_shadow(vec3 fragPosWs, shadowCubeMapSamplerType shadowMap, float fragDepth, vec3 lightPosition, float bias, const float maxSampleOffset, const uint samples)
{
	vec3 lightVec = normalize(fragPosWs - lightPosition);

	float biasDepth = fragDepth - bias;
	if(samples <= 1)
		return get_shadow_value(shadowMap, lightVec, biasDepth);
	float offset = max(maxSampleOffset * (1.0 - exp_scale_factor(fragDepth, 6.4)), MIN_SHADOW_SAMPLE_OFFSET);
	float nsamples = cbrt(float(samples));
	float inc = (offset / nsamples) * 2.0;

	float shadow = 0.0;
	for(float x = -offset; x < offset; x += inc) {
		for(float y = -offset; y < offset; y += inc) {
			for(float z = -offset; z < offset; z += inc)
				shadow += get_shadow_value(shadowMap, lightVec + vec3(x, y, z), biasDepth);
		}
	}
	shadow /= float(samples);
	return shadow;
}

const ShadowSettings g_cubeShadowSettings[5]
  = ShadowSettings[5](ShadowSettings(0, 0, 0.0, 0.0), ShadowSettings(2, 1, MIN_SHADOW_SAMPLE_OFFSET, 0.0), ShadowSettings(2, 8, MIN_SHADOW_CUBE_SAMPLE_OFFSET, 0.0), ShadowSettings(2, 27, MIN_SHADOW_CUBE_SAMPLE_OFFSET, 0.0), ShadowSettings(2, 64, MIN_SHADOW_CUBE_SAMPLE_OFFSET, 0.0));

float calculate_point_shadow(vec3 fragPosWs, shadowCubeMapSamplerType shadowMap, float fragDepth, vec3 lightPosition)
{
	ShadowSettings shadowSettings = g_cubeShadowSettings[CSPEC_SHADOW_QUALITY];
	float bias = 0.01 * exp_scale_factor(fragDepth, 0.9);
	return calculate_point_shadow(fragPosWs, shadowMap, fragDepth, lightPosition, bias, shadowSettings.maxSampleOffset, shadowSettings.samples);
}

#endif
