#ifndef F_FS_SHADOW_CSM_GLS
#define F_FS_SHADOW_CSM_GLS

#include "fs_shadow_spot.glsl"
#include "/common/inputs/csm.glsl"

// Required for CSM / PSSM -> standard in shader model 400 and above
//#extension GL_EXT_texture_array : enable

float calculate_csm_shadow(shadowMapSamplerType shadowMap, mat4 vp, vec4 worldCoord, float bias)
{
	vec4 shadowCoord = vp * worldCoord;
	shadowCoord.w = 1.0;
	shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
	shadowCoord.y = shadowCoord.y * 0.5 + 0.5;
	return calculate_spot_shadow(shadowMap, shadowCoord, 0.0); //,bias,0.0,1,0.0,0);
}

float calculate_csm_shadow(int index, vec4 worldCoord, float bias)
{
	return calculate_csm_shadow(csmTextures[index], u_csm.VP[index], worldCoord, bias); // Indexing by non-const int causes rendering artifacts on some hardware
	                                                                                    /*switch(index)
	{
		case 0:
			return calculate_csm_shadow(csmTextures[0],u_csm.VP[0],worldCoord,bias);
		case 1:
			return calculate_csm_shadow(csmTextures[1],u_csm.VP[1],worldCoord,bias);
		case 2:
			return calculate_csm_shadow(csmTextures[2],u_csm.VP[2],worldCoord,bias);
		default:
			return calculate_csm_shadow(csmTextures[3],u_csm.VP[3],worldCoord,bias);
	}*/
}

////////////////////////////////////

float get_shadow_value(sampler2DArray shadowMap, vec4 shadowCoord, vec2 offset, int layer)
{
#ifdef GL_EXT_texture_array
	float r = texture(shadowMap, shadowCoord.xyz + vec3(offset.x * u_renderSettings.shadowRatioX * shadowCoord.w, offset.y * u_renderSettings.shadowRatioY * shadowCoord.w, 0.0), shadowCoord.w /* bias */).x;
	return r / shadowCoord.w;
#else
	return 0.0; // Not supported by drivers
#endif
}

float calculate_csm_shadow(sampler2DArray shadowMap, vec4 worldCoord, float bias)
{
	int index = get_csm_cascade_index();
	if(index == -1)
		return 1.0;
	mat4 vp = u_csm.VP[index];
	vec4 shadowCoord = vp * worldCoord;
	//shadowCoord.w = shadowCoord.z *0.5 +0.5 -bias;
	shadowCoord.z -= bias;
	shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
	shadowCoord.y = shadowCoord.y * 0.5 + 0.5;

	float shadow = 0.0;
	for(float y = -SOFT_SHADOW_PCF_KERNEL_ITER; y <= SOFT_SHADOW_PCF_KERNEL_ITER; y += 1.0)
		for(float x = -SOFT_SHADOW_PCF_KERNEL_ITER; x <= SOFT_SHADOW_PCF_KERNEL_ITER; x += 1.0)
			shadow += get_shadow_value(shadowMap, shadowCoord, vec2(x, y), index);
	shadow /= float(SOFT_SHADOW_PCF_KERNEL);
	return shadow;
}

#endif
