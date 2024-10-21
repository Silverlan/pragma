#ifndef F_FS_LIGHTING_DIRECTIONAL_GLS
#define F_FS_LIGHTING_DIRECTIONAL_GLS

#include "/common/inputs/csm.glsl"
#include "/lighting/inputs/light_sources.glsl"

float get_directional_light_shadow_factor(uint lightIdx)
{
	LightSourceData light = get_light_source(lightIdx);
	vec4 shadowCoord = vec4(get_vertex_position_ws(), 1.0);
	float shadow = 1.0;
	int layer = get_csm_cascade_index();
	uint shadowMapIndex = light.shadowIndex;
	return (layer != -1 && shadowMapIndex != 0) ? calculate_shadow(layer, shadowCoord,
	                                                0.0 // Bias is already applied in shader pipeline
	                                                )
	                                            : 1.0;
}

float calc_directional_light_attenuation(uint lightIdx, vec3 vertNorm)
{
	LightSourceData light = get_light_source(lightIdx);
	vec3 N = -normalize(transpose(inverse(mat3(get_model_matrix()))) * vertNorm); // Is this correct?

	float lambertTerm = 1.0;
#if LIGHTING_USE_INCIDENCE_ANGLE == 1
	lambertTerm = max(dot(N, normalize(light.direction.xyz)), 0.0);
#endif
	float attenuation = lambertTerm;
	return attenuation * get_directional_light_shadow_factor(lightIdx);
}

vec3 calculate_directional_lighting(int lightIdx, vec3 fragLightColor, vec3 fragColor, vec3 specularColor, float cosTheta, vec3 vertNorm)
{
	LightSourceData light = get_light_source(lightIdx);
	vec3 N = -normalize(transpose(inverse(mat3(get_model_matrix()))) * vertNorm); // Is this correct?

	float attenuation = calc_directional_light_attenuation(lightIdx, vertNorm);
	fragLightColor += fragColor * light.color.a * cosTheta * attenuation + specularColor;

#if ENABLE_LIGHT_DEBUG == 1
#if SHADOW_PCF_ENABLED == 0
	/*if(CSPEC_DEBUG_MODE_ENABLED == 1)
	{
		vec4 lightVertPos = vec4(get_vertex_position_ws(),1.0);
		if((u_debug.flags &DEBUG_LIGHT_SHOW_SHADOW_MAP_DEPTH) != 0)
		{
			int layer = get_csm_cascade_index();
			vec4 shadowCoord = u_csm.VP[layer] *lightVertPos;
			float shadow = 1.0;
			if(layer != -1)
			{
				shadowCoord.w = shadowCoord.z *0.5 +0.5;
				shadowCoord.x = shadowCoord.x *0.5 +0.5;
				shadowCoord.y = shadowCoord.y *0.5 +0.5;
				shadow = texture(csmTextures[layer],vec2(shadowCoord.x,shadowCoord.y)).r;
			}
			fragLightColor += vec3(shadow,shadow,shadow);
		}
		else if((u_debug.flags &DEBUG_LIGHT_SHOW_FRAGMENT_DEPTH_SHADOW_SPACE) != 0)
		{
			vec4 shadowCoord = u_csm.VP[lightIdx] *lightVertPos;
			float shadow = shadowCoord.z;
			fragLightColor += vec3(shadow,shadow,shadow);
		}
	}*/
#endif

#endif
	return fragLightColor;
}

#endif