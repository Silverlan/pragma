#ifndef F_FS_LIGHTING_SPOT_GLS
#define F_FS_LIGHTING_SPOT_GLS

#include "/lighting/inputs/fs_shadow_maps.glsl"
#include "/lighting/inputs/fs_light_sources.glsl"

float get_spot_light_shadow_sample(uint lightIdx, bool dynamic, vec3 fragPosWs)
{
	LightSourceData light = get_light_source(lightIdx);
	if(light.shadowIndex == 0)
		return 1.0;
	vec4 depthCoord = get_light_vertex_position(lightIdx);
	depthCoord.z *= light.position.w; // Undo transformation from get_light_vertex_position()
	depthCoord = depthCoord / light.position.w;
	depthCoord.xy /= depthCoord.w;

	uint shadowMapIndex = light.shadowMapIndexDynamic;

	vec3 lightVec = normalize(fragPosWs - light.position.xyz);
	return (shadowMapIndex > 0) ? get_shadow_value(shadowMaps[shadowMapIndex - 1], depthCoord.xy, depthCoord.z) : 1.0;
}

float get_spot_light_shadow_factor(uint lightIdx, bool dynamic)
{
	LightSourceData light = get_light_source(lightIdx);
	if(light.shadowIndex == 0)
		return 1.0;
	vec4 depthCoord = get_light_vertex_position(lightIdx);
	depthCoord.z *= light.position.w; // Undo transformation from get_light_vertex_position()
	depthCoord = depthCoord / light.position.w;
	uint shadowMapIndex = light.shadowMapIndexDynamic;                                             //dynamic ? light.shadowMapIndexDynamic : light.shadowMapIndexStatic;
	return (shadowMapIndex > 0) ? calculate_shadow(shadowMaps[shadowMapIndex - 1], depthCoord, 0.0 /* bias is 0, since it's already been applied in the shader pipeline */
	                                )
	                            : 1.0;
}

float calc_spot_light_attenuation(uint lightIdx, vec3 fragPosWs)
{
	LightSourceData light = get_light_source(lightIdx);
	vec3 posFromWorldSpace = light.position.xyz - fragPosWs;
	vec3 dirToLight = normalize(posFromWorldSpace);
	vec3 lightDir = light.direction.xyz;
	float cos_cur_angle = dot(-dirToLight, lightDir);
	float cos_inner_cone_angle = cos(light.cutoffInner);
	float cos_outer_cone_angle = cos(light.cutoffOuter);
	if(cos_cur_angle <= cos_outer_cone_angle)
		return 0.0;
	float attenuation = calc_light_falloff(length(posFromWorldSpace), light.position.w); // Distance falloff
	attenuation = pow(attenuation, light.falloffExponent);

	// Apply shadow factor from static and dynamic objects
	attenuation *= get_spot_light_shadow_factor(lightIdx, true) * get_spot_light_shadow_factor(lightIdx, false);
	float cos_outer_minus_inner_angle = cos_inner_cone_angle - cos_outer_cone_angle;
	attenuation *= clamp((cos_cur_angle - cos_outer_cone_angle) / cos_outer_minus_inner_angle, 0.0, 1.0); // Cone falloff
	return attenuation;
}

vec3 calculate_spot_lighting(int lightIdx, int spotIdx, vec3 fragLightColor, vec3 fragColor, vec3 specularColor, float cosTheta, vec3 posFromWorldSpace, vec3 fragPosWs)
{
	LightSourceData light = get_light_source(lightIdx);
	vec3 dirToLight = normalize(posFromWorldSpace);

	float attenuation = calc_spot_light_attenuation(lightIdx, fragPosWs);

	vec3 diffuseLight = light.color.rgb * attenuation * light.color.a;
	fragLightColor += diffuseLight * fragColor * cosTheta + specularColor;

#if ENABLE_LIGHT_DEBUG == 1

#if SHADOW_PCF_ENABLED == 0
	/*if((u_debug.flags &DEBUG_LIGHT_SHOW_SHADOW_MAP_DEPTH) != 0)
	{
		float zFar = light.position.w;
		vec4 depthCoord = get_light_vertex_position(lightIdx);
		depthCoord /= depthCoord.w;
		float shadow = texture(shadowMaps[spotIdx],depthCoord.st).r;
		fragLightColor += vec3(shadow,shadow,shadow);
	}
	else if((u_debug.flags &DEBUG_LIGHT_SHOW_FRAGMENT_DEPTH_SHADOW_SPACE) != 0)
	{
		vec4 shadowCoord = get_light_vertex_position(lightIdx);
		fragLightColor += vec3(shadowCoord.z,shadowCoord.z,shadowCoord.z);
		float shadow = length(get_vertex_position_ws() -light.position.xyz) /light.position.w;
		fragLightColor += vec3(shadow,shadow,shadow);
	}*/
#endif

#endif
	return fragLightColor;
}

#endif