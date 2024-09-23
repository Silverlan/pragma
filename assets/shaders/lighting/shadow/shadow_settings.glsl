#ifndef F_SH_SHADOW_GLS
#define F_SH_SHADOW_GLS

#define SHADOW_PCF_ENABLED 0

#if SHADOW_PCF_ENABLED == 1
#define MIN_SHADOW_SAMPLE_OFFSET 0.0005
#define MIN_SHADOW_CUBE_SAMPLE_OFFSET 0.016
#else
#define MIN_SHADOW_SAMPLE_OFFSET 0.001
#define MIN_SHADOW_CUBE_SAMPLE_OFFSET 0.016
#endif

// Note: We can't use macros for sampler types (because it messes with Pragma's OpenGL shader implementation.)
// Instead, if the PCF flag above it changed, the types below have to be applied manually to the texture arrays in shaders/lighting/fs_shadow and shaders/modules/sh_csm!
#if SHADOW_PCF_ENABLED == 1
#define shadowMapSamplerType sampler2DShadow
#define shadowCubeMapSamplerType samplerCubeShadow
#else
#define shadowMapSamplerType sampler2D
#define shadowCubeMapSamplerType samplerCube
#endif

struct ShadowSettings {
	uint poissonSamples;
	uint samples;
	float maxSampleOffset;
	float sampleSpread;
};

#endif
