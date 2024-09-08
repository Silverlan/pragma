#ifndef F_FS_SHADOW_SPOT_GLS
#define F_FS_SHADOW_SPOT_GLS

#include "/math/poisson_disk.glsl"
#include "/math/math.glsl"
#include "shadow_settings.glsl"
#include "/programs/scene/pbr/fs_config.glsl"

#if SHADOW_PCF_ENABLED == 1
float get_shadow_value(sampler2DShadow shadowMap, vec2 uv, float fragDepth) { return texture(shadowMap, vec3(uv, fragDepth)).r; }
#else
float get_shadow_value(sampler2D shadowMap, vec2 uv, float fragDepth)
{
	float d = texture(shadowMap, uv).r;
	return (d < fragDepth) ? 0.0 : 1.0;
}
#endif

float calculate_spot_shadow(shadowMapSamplerType shadowMap, vec4 shadowCoord, float bias, const float maxSampleOffset, const uint samples, const float sampleSpread, const uint numPoissonSamples)
{
	shadowCoord.xy /= shadowCoord.w;

	float shadow = 0.0;
	shadowCoord.z -= bias;

	if(samples <= 1)
		return get_shadow_value(shadowMap, shadowCoord.xy, shadowCoord.z);

	const float offset = maxSampleOffset;
	const float nsamples = sqrt(float(samples));
	const float nsampleSpread = sampleSpread;
	const float inc = (offset / nsamples) * 2.0;

	//shadow = get_shadow_value(shadowMap,shadowCoord.xy,shadowCoord.z);
	uint testCount = 0;
	for(int i = 0; i < numPoissonSamples; ++i) {
		vec2 poissonOffset = get_poisson_disk_offset(i) * nsampleSpread;
		for(float x = -offset; x < offset; x += inc) {
			for(float y = -offset; y < offset; y += inc) {
				shadow += get_shadow_value(shadowMap, shadowCoord.xy + vec2(x, y) + poissonOffset, shadowCoord.z);
				++testCount;
			}
		}
	}
	shadow /= float(testCount); //float(samples *numPoissonSamples);
	return shadow;
}

const ShadowSettings g_shadowSettings[5] = ShadowSettings[5](ShadowSettings(0, 0, 0.0, 0.0), ShadowSettings(2, 4, MIN_SHADOW_SAMPLE_OFFSET, MIN_SHADOW_SAMPLE_OFFSET), ShadowSettings(2, 16, MIN_SHADOW_SAMPLE_OFFSET, 0.0001), ShadowSettings(2, 64, MIN_SHADOW_SAMPLE_OFFSET, 0.0005),
  ShadowSettings(2, 256, MIN_SHADOW_SAMPLE_OFFSET, 0.0005));

float calculate_spot_shadow(shadowMapSamplerType shadowMap, vec4 shadowCoord, float bias)
{
	ShadowSettings shadowSettings = g_shadowSettings[CSPEC_SHADOW_QUALITY];
	return calculate_spot_shadow(shadowMap, shadowCoord, bias, shadowSettings.maxSampleOffset, shadowSettings.samples, shadowSettings.sampleSpread, shadowSettings.poissonSamples);
}

float calculate_spot_shadow(shadowMapSamplerType shadowMap, vec4 shadowCoord) { return calculate_spot_shadow(shadowMap, shadowCoord, 0.01); }

////////////////////////////////////

// Obsolete?
/*
float calculate_spot_shadow(sampler2DShadow shadowMap,vec4 shadowCoord,float bias)
{
	float shadow = 0.0;
	shadowCoord.w += bias;
	return textureProj(shadowMap,vec4(
		shadowCoord.x +u_renderSettings.shadowRatioX *shadowCoord.w,
		shadowCoord.y +u_renderSettings.shadowRatioY *shadowCoord.w,
		shadowCoord.z,shadowCoord.w
	),bias);
}*/

#endif
