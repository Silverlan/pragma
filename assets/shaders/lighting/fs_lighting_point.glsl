#ifndef F_FS_LIGHTING_POINT_GLS
#define F_FS_LIGHTING_POINT_GLS

#include "/lighting/inputs/fs_light_sources.glsl"
#include "/lighting/inputs/fs_shadow_maps.glsl"

float get_point_light_shadow_sample(uint lightIdx, bool dynamic, vec3 fragPosWs)
{
	LightSourceData light = get_light_source(lightIdx);
	vec4 depthCoord = get_light_vertex_position(lightIdx);
	depthCoord.z = length(fragPosWs - light.position.xyz) / light.position.w;
	uint shadowMapIndex = dynamic ? light.shadowMapIndexDynamic : light.shadowMapIndexStatic;

	vec3 lightVec = normalize(fragPosWs - light.position.xyz);
	float bias = 0.01 * exp_scale_factor(depthCoord.z, 0.9);
	float biasDepth = depthCoord.z - bias;
	return (shadowMapIndex > 0) ? get_shadow_value(shadowCubemaps[shadowMapIndex - 1], lightVec, biasDepth) : 1.0;
}

float get_point_light_shadow_factor(uint lightIdx, bool dynamic, vec3 fragPosWs)
{
	LightSourceData light = get_light_source(lightIdx);
	vec4 depthCoord = get_light_vertex_position(lightIdx);
	depthCoord.z = length(fragPosWs - light.position.xyz) / light.position.w;
	uint shadowMapIndex = dynamic ? light.shadowMapIndexDynamic : light.shadowMapIndexStatic;
	return (shadowMapIndex > 0) ? calculate_shadow(fragPosWs, shadowCubemaps[shadowMapIndex - 1], depthCoord.z, light.position.xyz) : 1.0;
}

float calc_point_light_attenuation(uint lightIdx, vec3 fragPosWs)
{
	LightSourceData light = get_light_source(lightIdx);
	float lightDist = length(light.position.xyz - fragPosWs);
	float attenuation = calc_light_falloff(lightDist, light.position.w);
	attenuation = pow(attenuation, light.falloffExponent); // Distance falloff

	// Apply shadow factor from static and dynamic objects
	attenuation *= get_point_light_shadow_factor(lightIdx, true, fragPosWs) * get_point_light_shadow_factor(lightIdx, false, fragPosWs);
	return attenuation;
}

vec3 calculate_point_lighting(int lightIdx, int pointIdx, vec3 fragLightColor, vec3 fragColor, vec3 specularColor, float cosTheta, float lightDist, vec3 fragPosWs)
{
	LightSourceData light = get_light_source(lightIdx);

	float attenuation = calc_point_light_attenuation(lightIdx, fragPosWs);
	vec3 localColor = fragColor * light.color.a * cosTheta * attenuation + specularColor;
	fragLightColor.rgb += localColor.rgb;

#if ENABLE_LIGHT_DEBUG == 1

#if SHADOW_PCF_ENABLED == 0
	/*if((u_debug.flags &DEBUG_LIGHT_SHOW_SHADOW_MAP_DEPTH) != 0)
	{
		float zFar = light.position.w;
		float shadow = texture(shadowCubemaps[pointIdx],fragPosWs -light.position.xyz).r;
		fragLightColor += vec3(shadow,shadow,shadow);
	}
	else if((u_debug.flags &DEBUG_LIGHT_SHOW_FRAGMENT_DEPTH_SHADOW_SPACE) != 0)
	{
		float shadow = length(fragPosWs -light.position.xyz) /light.position.w;
		fragLightColor += vec3(shadow,shadow,shadow);
	}*/
#endif

#endif
	return fragLightColor;
}

#endif