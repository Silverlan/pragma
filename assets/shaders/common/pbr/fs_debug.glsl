#ifndef F_FS_PBR_DEBUG_GLS
#define F_FS_PBR_DEBUG_GLS

#define DEBUG_MODE_NONE 0
#define DEBUG_MODE_AO (DEBUG_MODE_NONE + 1)
#define DEBUG_MODE_ALBEDO (DEBUG_MODE_AO + 1)
#define DEBUG_MODE_METALNESS (DEBUG_MODE_ALBEDO + 1)
#define DEBUG_MODE_ROUGHNESS (DEBUG_MODE_METALNESS + 1)
#define DEBUG_MODE_DIFFUSE (DEBUG_MODE_ROUGHNESS + 1)
#define DEBUG_MODE_NORMAL (DEBUG_MODE_DIFFUSE + 1)
#define DEBUG_MODE_NORMAL_MAP (DEBUG_MODE_NORMAL + 1)
#define DEBUG_MODE_REFLECTANCE (DEBUG_MODE_NORMAL_MAP + 1)
#define DEBUG_MODE_IBL_PREFILTER_MAP (DEBUG_MODE_REFLECTANCE + 1)
#define DEBUG_MODE_IBL_IRRADIANCE_MAP (DEBUG_MODE_IBL_PREFILTER_MAP + 1)
#define DEBUG_MODE_EMISSION_MAP (DEBUG_MODE_IBL_IRRADIANCE_MAP + 1)
#define DEBUG_MODE_LIGHTMAP (DEBUG_MODE_EMISSION_MAP + 1)
#define DEBUG_MODE_LIGHTMAP_UV (DEBUG_MODE_LIGHTMAP + 1)
#define DEBUG_MODE_UNLIT (DEBUG_MODE_LIGHTMAP_UV + 1)
#define DEBUG_MODE_CSM_SHOW_CASCADES (DEBUG_MODE_UNLIT + 1)
#define DEBUG_MODE_SHADOW_MAP_DEPTH (DEBUG_MODE_CSM_SHOW_CASCADES + 1)
#define DEBUG_MODE_FORWARD_PLUS_HEATMAP (DEBUG_MODE_SHADOW_MAP_DEPTH + 1)
#define DEBUG_MODE_SPECULAR (DEBUG_MODE_FORWARD_PLUS_HEATMAP + 1)
#define DEBUG_MODE_LIGHTMAP_INDIRECT (DEBUG_MODE_SPECULAR + 1)
#define DEBUG_MODE_LIGHTMAP_DOMINANT (DEBUG_MODE_LIGHTMAP_INDIRECT + 1)

vec4 apply_debug_mode(uint materialFlags, uint debugMode, vec4 outputColor, vec2 texCoords, vec3 diffuseColor, vec3 specularColor, float metallic, float roughness, float aoFactor, float reflectance)
{
	if(debugMode == DEBUG_MODE_NONE)
		return outputColor;
	vec4 finalColor = outputColor;
	// Note: Using a switch-case here messes with the LunaGLASS optimizer.
	// Since these are only evaluated in debug mode, the overhead from the if-branches
	// (if there is one) doesn't really matter
	if(debugMode == DEBUG_MODE_AO) {
#ifdef MATERIAL_RMA_MAP_ENABLED
		float ao = fetch_rma_map(texCoords)[RMA_CHANNEL_AO];
#else
		float ao = 0.0;
#endif
		finalColor = mix(vec4(1, 1, 1, 1), vec4(ao, ao, ao, 1.0), aoFactor);
	}
	else if(debugMode == DEBUG_MODE_ALBEDO)
#ifdef MATERIAL_ALBEDO_MAP_ENABLED
		finalColor = texture(u_albedoMap, texCoords);
#else
		finalColor = vec4(1,1,1,1);
#endif
	else if(debugMode == DEBUG_MODE_METALNESS)
		finalColor = vec4(metallic, metallic, metallic, 1.0);
	else if(debugMode == DEBUG_MODE_ROUGHNESS)
		finalColor = vec4(roughness, roughness, roughness, 1.0);
	else if(debugMode == DEBUG_MODE_DIFFUSE)
		finalColor = vec4(diffuseColor, 1.0);
	else if(debugMode == DEBUG_MODE_SPECULAR)
		finalColor = vec4(specularColor, 1.0);
	else if(debugMode == DEBUG_MODE_NORMAL)
		finalColor = vec4(get_normal_from_map(texCoords, materialFlags), 1.0);
	else if(debugMode == DEBUG_MODE_NORMAL_MAP)
#ifdef MATERIAL_NORMAL_MAP_ENABLED
		finalColor = texture(u_normalMap, texCoords);
#else
		finalColor = vec4(1,1,1,1);
#endif
	else if(debugMode == DEBUG_MODE_REFLECTANCE)
		finalColor = vec4(reflectance, reflectance, reflectance, 1.0);
	else if(debugMode == DEBUG_MODE_IBL_PREFILTER_MAP)
		finalColor = texture(u_prefilterMap, normalize(get_vertex_position_ws() - u_renderSettings.posCam.xyz));
	else if(debugMode == DEBUG_MODE_IBL_IRRADIANCE_MAP)
		finalColor = texture(u_irradianceMap, normalize(get_vertex_position_ws() - u_renderSettings.posCam.xyz));
	else if(debugMode == DEBUG_MODE_EMISSION_MAP)
#ifdef MATERIAL_EMISSION_MAP_ENABLED
		finalColor = texture(u_emissionMap, texCoords);
#else
		finalColor = vec4(1,1,1,1);
#endif
	else if(debugMode == DEBUG_MODE_LIGHTMAP) {
		if(is_light_map_enabled()) {
			vec4 colLightMap = texture(u_lightMap, get_vertex_uv_lightmap());
			//colLightMap.rgb /= 255.0;
			finalColor = colLightMap.rgba;
		}
		else
			finalColor = vec4(0, 0, 0, 1);
	}
	else if(debugMode == DEBUG_MODE_LIGHTMAP_INDIRECT) {
		if(is_light_map_enabled() && is_indirect_light_map_enabled()) {
			vec4 colLightMap = texture(u_lightMapIndirect, get_vertex_uv_lightmap());
			//colLightMap.rgb /= 255.0;
			finalColor = colLightMap.rgba;
		}
		else
			finalColor = vec4(0, 0, 0, 1);
	}
	else if(debugMode == DEBUG_MODE_LIGHTMAP_DOMINANT) {
		if(is_light_map_enabled() && is_directional_light_map_enabled()) {
			vec4 colLightMap = texture(u_lightMapDominant, get_vertex_uv_lightmap());
			//colLightMap.rgb /= 255.0;
			finalColor = colLightMap.rgba;
		}
		else
			finalColor = vec4(0, 0, 0, 1);
	}
	else if(debugMode == DEBUG_MODE_LIGHTMAP_UV) {
		if(is_light_map_enabled())
			finalColor = vec4(get_vertex_uv_lightmap(), 0, 1);
		else
			finalColor = vec4(0, 0, 0, 1);
	}
	else if(debugMode == DEBUG_MODE_CSM_SHOW_CASCADES)
		finalColor = debug_get_csm_cascade_color(finalColor);
	return finalColor;
}

#endif
